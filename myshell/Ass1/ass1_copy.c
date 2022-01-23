#include "c_keywords.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

// 存储文件内容
char Document[10010];
// 存储行数
int lines;
// 文件中的所有字符数
int char_count;

// 打开并读取文件
void OpenAndReadFile() {
    printf("Please input filename: ");
    char filename[30];
    scanf("%s", filename);
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Open file error");
        return;
    }
    char_count = 0;
    char ch;
    // 一直读到文件尾(EOF)
    while (1) {
        ch = fgetc(fp);
        if (feof(fp)) { // feof用于判断是否到达文件尾
            break;
        }
        if (ch != '\n' && ch != '\r') {
            Document[char_count] = ch;
        } else {
            Document[char_count] = ' ';
        }
        char_count++;
    }
}

// 判断是否为分隔符
int is_punctuation(char ch) {
    for (int i = 0; i < 4; i++) {
        if (ch == punctuation_marks[i])
            return 1;
    }
    return 0;
}

// 判断是否为关键字
int is_keyword(char *word) {
    for (int i = 0; i < 32; i++) {
        if (strcmp(word, c_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

// 解析文件中的关键字 符号
void ParseFile() {
    char word_temp[50]; // 用于暂存单词 比较是否为关键字
    char sentence_temp[1010]; // 用于暂存句子
    int idx_word = 0;
    int idx_sentence = 0;

    for (int i = 0; i < char_count; i++) {
        if (!(Document[i] == ' ' && idx_sentence == 0)) {
            sentence_temp[idx_sentence++] = Document[i];
        }

        if (isalpha(Document[i])) { // 将各个单词拆分出来
            word_temp[idx_word++] = Document[i];
            if (i + 1 < char_count && (is_punctuation(Document[i + 1]) || Document[i + 1] == ' ')) {
                word_temp[idx_word] = '\0';
                // puts(word_temp);  // --> 可以在这里输出 看看拆分的结果
                if (is_keyword(word_temp)) { // 检测到keywords
                    // 去掉当前存储的句子中的keywords
                    while ((!is_punctuation(sentence_temp[idx_sentence]) && sentence_temp[idx_sentence] != ' ') && idx_sentence - 1 >= 0)
                        idx_sentence--;
                    sentence_temp[idx_sentence] = '\0';
                    // 输出当前存储的句子
                    if (idx_sentence != 0) {
                        puts(sentence_temp);
                        lines++;
                    }
                    idx_sentence = 0;

                    // 如果文本的下个字符是分隔符 那么关键词需要连着分隔符一起输出
                    if (is_punctuation(Document[i + 1])) {
                        word_temp[idx_word++] = Document[i + 1];
                        word_temp[idx_word] = '\0';
                        i++;
                    }
                    puts(word_temp);
                    lines++;
                }
                idx_word = 0;
            }
        }
        // 如果遇到分隔符 但是之前并没有遇到关键词 输出句子
        if (is_punctuation(Document[i]) && idx_sentence != 0) {
            sentence_temp[idx_sentence] = '\0';
            puts(sentence_temp);
            idx_sentence = 0;
            lines++;
        }
    }
}

int main() {
    OpenAndReadFile();
    ParseFile();
    printf("Total number of lines is: %d\n", lines);
}
