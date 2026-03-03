/*написать программу генерации индивидуальных заданий по теме 
"Машинное представление вещественных чисел". Использовать 
разработанные ранее функции перевода 32 и 64 битных чисел 
в машинное представление и обратно. На входе текстовый файл 
с параметрами: N вариантов, K заданий, разрядность (32/64), 
диапазон [a,b], точность P знаков. На выходе для каждого 
варианта 2 файла в формате .md: задание и ответы.*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

//функции перевода 32-битных чисел (из предыдущего курса)
void fl(int *a, float ch) {
    unsigned int *ptr=(unsigned int *)&ch;
    unsigned int bits=*ptr;
    for (int i=0; i<32; i++) {
        unsigned int stepen=1;
        for (int j=0; j<i; j++) stepen=stepen*2;
        a[31-i]=(bits/stepen)%2;
    }
}

float decf(int *a) {
    int s=a[0];
    int exp=0;
    float norm=1.0f;
    for (int i = 1; i <= 8; i++) {
        exp=exp*2+a[i];
    }
    for (int i=9; i<32; i++) {
        if (a[i]) {
            norm+=powf(2.0f, -(float)(i - 8));
        }
    }
    if (exp==0) {
        exp=-126;
        norm-=1.0f;
    } else if (exp==255) {
        if (norm==1.0f) {
            return (s==0) ? INFINITY : -INFINITY;
        } else {
            return NAN;
        }
    } else {
        exp-=127;
    }
    float res=powf(-1.0f, (float)s)*norm*powf(2.0f, (float)exp);
    return res;
}

//функции перевода 64-битных чисел (из предыдущего курса)
void dou(int *a, double ch) {
    unsigned long long *ptr=(unsigned long long *)&ch;
    unsigned long long bits=*ptr;
    for (int i=0; i<64; i++) {
        unsigned long long stepen=1;
        for (int j=0; j<i; j++) stepen=stepen*2;
        a[63-i]=(bits/stepen)%2;
    }
}

double decd(int *a) {
    int s=a[0];
    long long exp=0;
    double norm=1.0;
    for (int i=1; i<=11; i++) {
        exp=exp*2+a[i];
    }
    for (int i=12; i<64; i++) {
        if (a[i]) {
            norm+=pow(2.0, -(double)(i-11));
        }
    }
    if (exp==0) {
        exp=-1022;
        norm-=1.0;
    } else if (exp==2047) {
        if (norm==1.0) {
            return (s==0) ? INFINITY : -INFINITY;
        } else {
            return NAN;
        }
    } else {
        exp-=1023;
    }
    double res=pow(-1.0, (double)s)*norm*pow(2.0, (double)exp);
    return res;
}

//биты в строку для 32 бит
void bits_to_str_32(int *bits, char *str) {
    for (int i=0; i<32; i++) {
        str[i]=bits[i]+'0';
    }
    str[32]='\0';
}

//биты в строку для 64 бит
void bits_to_str_64(int *bits, char *str) {
    for (int i=0; i<64; i++) {
        str[i]=bits[i]+'0';
    }
    str[64]='\0';
}

//создаем директорию
void create_dir(const char *path) {
    #ifdef _WIN32
        mkdir(path);
    #else
        mkdir(path, 0755);
    #endif
}

//случайное число от a до b с точностью p знаков
double random_double(double a, double b, int p) {
    double r=a+(b-a)*(rand()/(RAND_MAX+1.0));
    double mult=pow(10.0, p);
    return round(r*mult)/mult;
}

//форматируем число в строку
void format_double(char *buffer, double value, int p) {
    char format[20];
    sprintf(format, "%%.%df", p);
    sprintf(buffer, format, value);
}

//читаем конфиг из файла
int read_config(const char *filename, int *n, int *k, int *bitness, 
                double *a, double *b, int *p) {
    FILE *file=fopen(filename, "r");
    if (!file) return 0;
    fscanf(file, "%d %d %d %lf %lf %d", n, k, bitness, a, b, p);
    fclose(file);
    return 1;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int n, k, bitness, p;
    double a, b;
    printf("Генератор заданий по вещественным числам\n");
    if (argc>1) {
        if (read_config(argv[1], &n, &k, &bitness, &a, &b, &p)) {
            printf("Конфигурация из файла: %s\n", argv[1]);
            printf("N=%d, K=%d, разрядность=%d, [%g,%g], P=%d\n\n", 
                   n, k, bitness, a, b, p);
        } else {
            printf("Ошибка: не могу прочитать %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("Введите параметры:\n");
        printf("Количество вариантов (n): ");
        scanf("%d", &n);
        printf("Заданий на студента (k): ");
        scanf("%d", &k);
        printf("Разрядность (32/64): ");
        scanf("%d", &bitness);
        printf("Диапазон (a b): ");
        scanf("%lf %lf", &a, &b);
        printf("Знаков после запятой (p): ");
        scanf("%d", &p);
        printf("\n");
    }
    
    if (n<=0 || k<=0 || p<0 || a>b) {
        printf("Ошибка: неверные параметры\n");
        return 1;
    }
    if (bitness!=32 && bitness!=64) {
        printf("Ошибка: разрядность должна быть 32 или 64\n");
        return 1;
    }
    
    create_dir("задание");
    create_dir("проверка");
    
    printf("Генерация %d вариантов...\n", n);
    printf("Файлы сохраняются в папки 'задание' и 'проверка'\n\n");
    
    int *bits32=(int*)malloc(32*sizeof(int));
    int *bits64=(int*)malloc(64*sizeof(int));
    if (!bits32 || !bits64) {
        printf("Ошибка: не могу выделить память\n");
        return 1;
    }
    
    for (int var=1; var<=n; var++) {
        double *numbers=(double*)malloc(k*sizeof(double));
        for (int i=0; i<k; i++) {
            numbers[i]=random_double(a, b, p);
        }
        
        char task_filename[100];
        sprintf(task_filename, "задание/var_%d.md", var);
        FILE *task_file=fopen(task_filename, "w");
        if (task_file) {
            fprintf(task_file, "# Вариант %d\n\n", var);
            fprintf(task_file, "## Перевести числа в машинное представление\n\n");
            fprintf(task_file, "| N | Число |\n");
            fprintf(task_file, "|--|-------|\n");
            for (int i=0; i<k; i++) {
                char num_str[50];
                format_double(num_str, numbers[i], p);
                fprintf(task_file, "| %d | %s |\n", i+1, num_str);
            }
            fclose(task_file);
        }
        
        char check_filename[100];
        sprintf(check_filename, "проверка/var_%d.md", var);
        FILE *check_file=fopen(check_filename, "w");
        if (check_file) {
            fprintf(check_file, "# Вариант %d (ответы)\n\n", var);
            fprintf(check_file, "## Проверка перевода\n\n");
            fprintf(check_file, "| N | Число | Машинное представление (биты) | Ошибка |\n");
            fprintf(check_file, "|---|-------|-------------------------------|--------|\n");
           
            for (int i=0; i<k; i++) {
                char num_str[50];
                char bits_str[100]="";
                char error_str[50];
                
                format_double(num_str, numbers[i], p);
                double converted=numbers[i];
                double error=0.0;
                
                if (bitness==32) {
                    fl(bits32, (float)numbers[i]);
                    bits_to_str_32(bits32, bits_str);
                    converted=decf(bits32);
                } else {
                    dou(bits64, numbers[i]);
                    bits_to_str_64(bits64, bits_str);
                    converted=decd(bits64);
                }
                
                error=fabs(numbers[i]-converted);
                sprintf(error_str, "%.2e", error);
                
                fprintf(check_file, "| %d | %s | %s | %s |\n", 
                        i+1, num_str, bits_str, error_str);
            }
            fclose(check_file);
        }
        
        free(numbers);
        printf("Вариант %d готов: задание/var_%d.md и проверка/var_%d.md\n", var, var, var);
    }
    
    free(bits32);
    free(bits64);
    
    printf("\nГотово!\n");
    printf("Все файлы сохранены в папках 'задание' и 'проверка'\n");
    
    return 0;
}
