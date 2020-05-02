#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>
#include <math.h>
#include <unistd.h>

typedef enum Method{
    SIGN,
    BLOCK,
    INTERLEAVED
} Method;

const int MAX_LINE_LEN = 2048;
const int NUMBER_OF_PIXELS = 256;
int width;
int height;
int number_of_threads;
int **image;
int **histograms;

void error_exit(char *message){
    printf("ERROR: %s\n", message);
    exit(EXIT_FAILURE);
}

void load_image(char *file_name){
    FILE *file = fopen(file_name, "r");
    if (file == NULL) error_exit("Cannot open file");

    char buffer[MAX_LINE_LEN];
    int line_number = 0;
    int image_max_value;

    // read basic informations
    while(line_number < 3 && fgets(buffer, MAX_LINE_LEN, file) != NULL){

        // ignore comments
        if(buffer[0] == '#') continue;

        // check color mode
        if(line_number == 0 && strncmp("P2", buffer, 2)){
            fclose(file);
            error_exit("Wrong image color mode. Expected [P2].");
        }

        // get width and height
        else if(line_number == 1){
            if (sscanf(buffer, "%d %d\n", &width, &height) != 2){
                fclose(file);
                error_exit("Cannot get width and height of image.");
            }

            image = (int **) calloc(height, sizeof(int *));
            for (int i = 0; i < height; i++)
                image[i] = (int *) calloc(width, sizeof(int));
        }
        
        // get image max value
        else if(line_number == 2)
        {
            if (sscanf(buffer, "%d\n", &image_max_value) != 1){
                fclose(file);
                error_exit("Cannot get image max value.");
            }
        }
        line_number ++;
    }

    if (line_number != 3){
        fclose(file);
        error_exit("Cannot parse image.");
    }

    // fill image array
    int val;
    for (int row = 0; row < height; row ++){
        for (int col = 0; col < width; col ++){
            fscanf(file, "%d", &val);
            image[row][col]=val;
        }
    }
    fclose(file);
}

int calculate_time (clock_t start_time){
    clock_t end_time = clock();
    return (1e6 * (end_time - start_time) / sysconf(_SC_CLK_TCK));
}

void *sign_thread(void *arg){
    int index = *((int*)arg);

    clock_t start_time = clock();

    int min_value = index * ceil((double) NUMBER_OF_PIXELS / number_of_threads);
    int max_value = (index + 1) * ceil((double) NUMBER_OF_PIXELS / number_of_threads) - 1;

    if (max_value > NUMBER_OF_PIXELS - 1)
        max_value = NUMBER_OF_PIXELS - 1;
    
    for (int row = 0; row < height; row ++){
        for (int col = 0; col < width; col ++){
            if (image[row][col] >= min_value && image[row][col] <= max_value)
                histograms[index][image[row][col]] ++;
        }
    }

    int* result_time = (int*)malloc(sizeof(int));
    *result_time = calculate_time(start_time);
    return result_time;
}

void *block_thread(void *arg){
    int index = *((int*)arg);

    clock_t start_time = clock();

    int start_col = index * ceil((double) width / number_of_threads);
    int end_col = (index + 1) * ceil((double) width / number_of_threads);
    if (end_col > width)
        end_col = width;

    for (int row = 0; row < height; row ++){
        for (int col = start_col; col < end_col; col ++){
            histograms[index][image[row][col]] ++;
        }
    }

    int* result_time = (int*)malloc(sizeof(int));
    *result_time = calculate_time(start_time);
    return result_time;
}

void *interleaved_thread(void *arg){
    int index = *((int*)arg);

    clock_t start_time = clock();

    int start_col = index;
    int step = number_of_threads;

    for (int row = 0; row < height; row ++){
        for (int col = start_col; col < width; col += step){
            histograms[index][image[row][col]] ++;
        }
    }

    int* result_time = (int*)malloc(sizeof(int));
    *result_time = calculate_time(start_time);
    return result_time;
}

void free_memory(){
    for (int i = 0; i < height; i++)
        free(image[i]);
    free(image);

    for (int i = 0; i < number_of_threads; i++)
        free(histograms[i]);
    free(histograms);
}

// save histogram to file
void save_result_to_file(char* file_name){
    FILE *file = fopen(file_name, "w+");
    if (file == NULL){
        free_memory();
        error_exit("Cannot save result to file.");
    }

    fprintf(file, "P2\n");
    fprintf(file, "Width: %d, Height: %d\n", width, height);
    fprintf(file, "Number of threads: %d\n", number_of_threads);
    fprintf(file, "-------------------------\n");

    for (int i = 0; i < NUMBER_OF_PIXELS; i++){
        int value = 0;
        for (int j = 0; j < number_of_threads; j++)
            value += histograms[j][i];
        fprintf(file, "%d: %d\n", i, value);
    }
    fclose(file);
}

// save times to Times.txt
void save_times(char *file_name, char* method, pthread_t * threads, clock_t start_time){
    FILE* times = fopen(file_name, "a");
    if (times == NULL)
        error_exit("Cannot open file");
    
    fprintf(times, "\n______ %s => %d threads ______\n", method, number_of_threads);
    for (int i = 0; i < number_of_threads; i++){
        int *time;
        pthread_join(threads[i], (void **) &time);
        printf("Thread: %ld, time: %d [us]\n", threads[i], *time);
        fprintf(times, "Thread: %d, time: %d [us]\n", i + 1, *time);
    }

    int time = calculate_time(start_time);
    printf("Full time: %d [us]\n\n", time);
    fprintf(times, "Full time: %d [us]\n", time);
}

int main(int argc, char **argv){
    if (argc != 5){
        error_exit("Wrong number of arguments. Expected ./main [number of threads] [split method: sign / block / interleaved] [input file with image] [output file]");
    }

    number_of_threads = atoi(argv[1]);
    Method split_method;

    if (!strcmp("sign", argv[2])){
        split_method = SIGN;
    }
    else if (!strcmp("block", argv[2])){
        split_method = BLOCK;
    }
    else if (!strcmp("interleaved", argv[2])){
        split_method = INTERLEAVED;
    }
    else{
        error_exit("Wrong split method");
    }

    char* input_file = argv[3];
    char* output_file = argv[4];

    load_image(input_file);

    histograms = (int **) calloc(number_of_threads, sizeof(int*));
    for (int i = 0; i < number_of_threads; i ++)
        histograms[i] = (int*)calloc(NUMBER_OF_PIXELS, sizeof(int));
    
    clock_t start_time = clock();

    pthread_t * threads = (pthread_t*)calloc(number_of_threads, sizeof(pthread_t));
    int *indexes = (int*)calloc(number_of_threads, sizeof(int));

    for (int i = 0; i < number_of_threads; i++){
        indexes[i] = i;
        if (split_method == SIGN)
            pthread_create(&threads[i], NULL, &sign_thread, &indexes[i]);
        else if(split_method == BLOCK)
            pthread_create(&threads[i], NULL, &block_thread, &indexes[i]);
        else 
            pthread_create(&threads[i], NULL, &interleaved_thread, &indexes[i]);
    }

    save_times("Times.txt", argv[2], threads, start_time);
    save_result_to_file(output_file);
    free_memory();
    
    return 0;
}