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
        Document[char_count] = ch;
        if (Document[char_count] == '\n' || Document[char_count] == '\r') {
            lines++;
        }
        char_count++;
    }
}

int is_punctuation(char ch) {
    for (int i = 0; i < 4; i++) {
        if (ch == punctuation_marks[i])
            return 1;
    }
    return 0;
}

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
    memset(word_temp, 0, sizeof(word_temp));
    memset(sentence_temp, 0, sizeof(sentence_temp));
    for (int i = 0; i < char_count; i++) {
        if (Document[i] != '\n' && Document[i] != '\r')
            sentence_temp[idx_sentence++] = Document[i];
        if (isalpha(Document[i])) { // ���������ʲ�ֳ���
            word_temp[idx_word++] = Document[i];
            if (i + 1 < char_count &&
                (is_punctuation(Document[i + 1]) || Document[i + 1] == ' ' ||
                 Document[i + 1] == '\n' || Document[i + 1] == '\r')) {
                word_temp[idx_word] = '\0';
                // puts(word_temp);  // --> ������������� ������ֵĽ��
                if (is_keyword(word_temp)) { // ��⵽keywords
                    if (is_punctuation(Document[i + 1])) {
                        word_temp[idx_word++] = Document[i + 1];
                        word_temp[idx_word] = '\0';
                    }
                    while ((is_punctuation(Document[i + 1]) ||
                        Document[i + 1] == ' ' || Document[i + 1] == '\n' ||
                        Document[i + 1] == '\r') && i + 1 < char_count)
                        i++;
                    while ((!is_punctuation(sentence_temp[idx_sentence]) ||
                           !sentence_temp[idx_sentence] == ' ' ||
                           !sentence_temp[idx_sentence] == '\n' ||
                           !sentence_temp[idx_sentence] == '\r') && idx_sentence - 1 > 0)
                            idx_sentence--;
                    sentence_temp[--idx_sentence] = '\0';
                    if (idx_sentence != 0)
                        printf("----> %d %s\n", idx_sentence, sentence_temp);
                    puts(word_temp);
                    idx_sentence = 0;
                    memset(sentence_temp, 0, sizeof(sentence_temp));
                }
                idx_word = 0;
                memset(word_temp, 0, sizeof(word_temp));
            }
        }
        if (idx_sentence) {
            puts(sentence_temp);
            idx_sentence = 0;
            memset(sentence_temp, 0, sizeof(sentence_temp));
        }
    }

}

int main() {
    OpenAndReadFile();
    ParseFile();
    printf("Total number of lines is: %d\n", lines);
}
