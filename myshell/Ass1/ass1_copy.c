#include "c_keywords.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

// �洢�ļ�����
char Document[10010];
// �洢����
int lines;
// �ļ��е������ַ���
int char_count;

// �򿪲���ȡ�ļ�
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
    // һֱ�����ļ�β(EOF)
    while (1) {
        ch = fgetc(fp);
        if (feof(fp)) { // feof�����ж��Ƿ񵽴��ļ�β
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

// �ж��Ƿ�Ϊ�ָ���
int is_punctuation(char ch) {
    for (int i = 0; i < 4; i++) {
        if (ch == punctuation_marks[i])
            return 1;
    }
    return 0;
}

// �ж��Ƿ�Ϊ�ؼ���
int is_keyword(char *word) {
    for (int i = 0; i < 32; i++) {
        if (strcmp(word, c_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

// �����ļ��еĹؼ��� ����
void ParseFile() {
    char word_temp[50]; // �����ݴ浥�� �Ƚ��Ƿ�Ϊ�ؼ���
    char sentence_temp[1010]; // �����ݴ����
    int idx_word = 0;
    int idx_sentence = 0;

    for (int i = 0; i < char_count; i++) {
        if (!(Document[i] == ' ' && idx_sentence == 0)) {
            sentence_temp[idx_sentence++] = Document[i];
        }

        if (isalpha(Document[i])) { // ���������ʲ�ֳ���
            word_temp[idx_word++] = Document[i];
            if (i + 1 < char_count && (is_punctuation(Document[i + 1]) || Document[i + 1] == ' ')) {
                word_temp[idx_word] = '\0';
                // puts(word_temp);  // --> ������������� ������ֵĽ��
                if (is_keyword(word_temp)) { // ��⵽keywords
                    // ȥ����ǰ�洢�ľ����е�keywords
                    while ((!is_punctuation(sentence_temp[idx_sentence]) && sentence_temp[idx_sentence] != ' ') && idx_sentence - 1 >= 0)
                        idx_sentence--;
                    sentence_temp[idx_sentence] = '\0';
                    // �����ǰ�洢�ľ���
                    if (idx_sentence != 0) {
                        puts(sentence_temp);
                        lines++;
                    }
                    idx_sentence = 0;

                    // ����ı����¸��ַ��Ƿָ��� ��ô�ؼ�����Ҫ���ŷָ���һ�����
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
        // ��������ָ��� ����֮ǰ��û�������ؼ��� �������
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
