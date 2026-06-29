/*
 * 文件名: main.cpp
 * 功能: 基于贪心算法的 Huffman 编码解码系统实现
 */
#include "huffman.h"
#include <iostream>
#include <cmath>//用于计算熵和平均码长
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

#define ASCII_SIZE 256

// 辅助函数声明
long CountFrequency(const char *filename, int *freq_array);//统计字符频率并返回总字符数
void EncodeSourceFile(const char *src_file, const char *dst_file, HuffmanCode HC, int *char_map, int n);//编码源文件并写入目标文件
double CalculateEntropy(HuffmanTree HT, int n, long total_chars);//计算信源熵
double CalculateAverageCodeLength(HuffmanTree HT, HuffmanCode HC, int n, long total_chars);//计算平均码长
void CalculateCompressionRatio(const char *src_file, const char *encoded_file);//计算压缩率
void ShowInformationTheoryMetrics(HuffmanTree HT, HuffmanCode HC, int n, long total_chars);//显示信息论定量分析结果
bool VerifyFileIntegrity(const char *file1, const char *file2);//验证文件完整性 ，返回是否一致

void ShowMenu() {
    printf("\n======================================\n");
    printf("      Huffman 编码解码系统          \n");
    printf("======================================\n");
    printf("  1. 初始化并生成 Huffman 树 (读取 source.txt)\n");
    printf("  2. 查看 Huffman 编码表\n");
    printf("  3. 编码文件 (source.txt -> encoded.txt)\n");
    printf("  4. 解码文件 (encoded.txt -> decoded.txt)\n");
    printf("  5. 信息论定量分析 (信源熵、平均码长、编码效率、压缩率)\n");
    printf("  6. 无失真校验 (对比source.txt与decoded.txt)\n");
    printf("  0. 退出系统\n");
    printf("======================================\n");
    printf("请选择操作 [0-6]: ");
}//显示菜单并提示用户选择

int main() {
    // 设置终端编码为UTF-8
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);//设置终端编码为UTF-8，确保中文字符正常显示
    #endif
    
    // 数据结构初始化和菜单循环控制，确保在菜单系统中正常工作
    HuffmanTree HT = NULL;
    HuffmanCode HC;
    HC.chars = NULL;
    HC.HCPointer = NULL;
    int char_map[ASCII_SIZE] = {0};//字符映射表，记录每个字符的编码索引
    int n = 0; //有效字符数
    long total_chars = 0;//总字符数
    bool is_initialized = false;     //是否已初始化编码系统

    const char *SOURCE_FILE = "source.txt";//源文件路径
    const char *ENCODED_FILE = "encoded.txt";//编码文件路径
    const char *DECODED_FILE = "decoded.txt";//解码文件路径

    int choice;
    while (true) {
        ShowMenu();
        if (scanf("%d", &choice) != 1) {
            printf("输入格式错误，请重新输入。\n");
            while (getchar() != '\n'); // 清空输入缓冲区
            continue;
        }

        if (choice == 0) break;

        switch (choice) {
            case 1: { // 1. 初始化编码
                // 如果之前已经初始化，释放内存
                if (is_initialized) {
                    FreeHuffman(HT, HC, n);
                    for(int i=0; i<ASCII_SIZE; i++) char_map[i] = 0;//清空字符映射表
                    n = 0;//重置有效字符数
                }

                int freq[ASCII_SIZE] = {0};//字符频率数组，记录每个字符的出现次数
                long temp_total_chars = CountFrequency(SOURCE_FILE, freq);//统计字符频率并返回总字符数
                if (temp_total_chars <= 0) {
                    printf("错误: 无法读取源文件，文件为空。\n");
                    break;
                }

                for (int i = 0; i < ASCII_SIZE; i++) if (freq[i] > 0) n++;//统计有效字符数，即出现次数大于0的字符数

                int *weights = (int *)malloc(n * sizeof(int));//权重数组，记录每个字符的出现次数
                char *active_chars = (char *)malloc(n * sizeof(char));//活动字符数组，记录每个字符的编码索引
                int k = 0;//有效字符数的索引
                for (int i = 0; i < ASCII_SIZE; i++) {
                    if (freq[i] > 0) {
                        weights[k] = freq[i];
                        active_chars[k] = (char)i;
                        char_map[i] = k + 1;
                        k++;
                    }
                }//每一个weight里存的是出现过至少一次的字符的出现次数，active_chars里存的是出现过至少一次的字符的编码索引，char_map里存的是每个字符的编码索引

                CreateHuffmanTree(HT, weights, n);
                GenerateHuffmanCodes(HT, HC, active_chars, n);
                
                free(weights);
                free(active_chars);
                
                is_initialized = true;
                total_chars = temp_total_chars;
                printf("Huffman 树生成成功! (总字符数: %ld)\n", total_chars);
                break;
            }

            case 2: { // 2. 查看编码表
                if (!is_initialized) {
                    printf("请先初始化编码。\n");
                } else {
                    printf("\n--- Huffman 编码表 ---\n");
                    for (int i = 1; i <= n; i++) {
                        char c = HC.chars[i];
                        if (c == '\n') printf("'\\n': %s\n", HC.HCPointer[i]);
                        else if (c == ' ') printf("' ': %s\n", HC.HCPointer[i]);
                        else printf("'%c' : %s\n", c, HC.HCPointer[i]);
                    }
                }
                break;
            }

            case 3: { // 3. 编码文件
                if (!is_initialized) {
                    printf("请先初始化编码\n");
                } else {
                    EncodeSourceFile(SOURCE_FILE, ENCODED_FILE, HC, char_map, n);
                }
                break;
            }

            case 4: { // 4. 解码文件
                if (!is_initialized) {
                    printf("请先初始化编码\n");
                } else {
                    printf("正在解码...\n");
                    if (DecodeFile(HT, HC, ENCODED_FILE, DECODED_FILE)) {
                        printf(">>> 解码完成! 请在%s中查看\n", DECODED_FILE);
                    }
                }
                break;
            }

            case 5: { // 5. 信息论定量分析
                ShowInformationTheoryMetrics(HT, HC, n, total_chars);
                break;
            }

            case 6: { // 6. 无失真校验
                printf("正在进行无失真校验...\n");
                if (VerifyFileIntegrity(SOURCE_FILE, DECODED_FILE)) {
                    printf("\n✓ 校验通过! 源文件与解压文件完全一致 (无失真)\n");
                } else {
                    printf("\n✗ 校验失败! 源文件与解压文件存在差异\n");
                }
                break;
            }

            default:
                printf("选择无效，请重新输入。\n");
        }
    }

    // 退出前释放资源
    if (is_initialized) FreeHuffman(HT, HC, n);
    printf("感谢使用，再见！\n");
    return 0;
}

// 统计字符频率并返回总字符数
long CountFrequency(const char *filename, int *freq_array) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;
    for (int i = 0; i < ASCII_SIZE; i++) freq_array[i] = 0;//清空字符频率数组
    long count = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch >= 0 && ch < ASCII_SIZE) {
            // 保留英文字符（大小写）、空格、换行符和常见标点符号
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || 
                ch == ' ' || ch == '\n' || ch == '.' || ch == ',' || 
                ch == '!' || ch == '?' || ch == ';' || ch == ':' || 
                ch == '"' || ch == '\'' || ch == '(' || ch == ')' || 
                ch == '-' || ch == '_' || ch == '=' || ch == '+' || 
                ch == '[' || ch == ']' || ch == '{' || ch == '}' || 
                ch == '|' || ch == '\\' || ch == ';' || ch == ':' || 
                ch == '<' || ch == '>' || ch == '/' || ch == '?') {
                freq_array[ch]++;//统计字符频率
                count++;//统计总字符数
            }
        }
    }
    fclose(fp);
    return count;
}



void EncodeSourceFile(const char *src_file, const char *dst_file, HuffmanCode HC, int *char_map, int n) {
    FILE *fin = fopen(src_file, "r");//打开源文件
    FILE *fout = fopen(dst_file, "wb"); // 以二进制模式打开目标文件，用于写入编码后的数据
    if (!fin || !fout) {
        printf("错误: 无法打开文件。\n");
        return;
    }
    
    // 写入字符数量
    fwrite(&n, sizeof(int), 1, fout);
    
    // 写入字符-频率表
    int freq[ASCII_SIZE] = {0};//字符频率数组，用于存储每个字符的出现次数
    CountFrequency(src_file, freq);//统计字符频率
    for (int i = 1; i <= n; i++) {
        char c = HC.chars[i];
        int frequency = freq[(unsigned char)c];
        fwrite(&c, sizeof(char), 1, fout);//写入字符
        fwrite(&frequency, sizeof(int), 1, fout);//写入字符频率
    }
    
    unsigned char buffer = 0;//用于存储编码后的二进制数据
       int bit_count = 0;//用于记录当前缓冲区已使用的位数
    int ch;
    
    while ((ch = fgetc(fin)) != EOF) {
        int index = char_map[ch];
        if (index != 0) {
            char *code = HC.HCPointer[index];
            for (int i = 0; code[i] != '\0'; i++) {
                buffer <<= 1;//将缓冲区左移一位，为下一位做准备
                if (code[i] == '1') {
                    buffer |= 1;//如果当前位为1，将缓冲区的最低位设为1
                }
                bit_count++;//记录当前位数
                // 当缓冲区满8位时，写入到目标文件
                if (bit_count == 8) {
                    fwrite(&buffer, sizeof(unsigned char), 1, fout);
                    buffer = 0;
                    bit_count = 0;//清空缓冲区，准备写入下一位数据
                }
            }
        }
    }
    
    // 处理最后不足8位的情况
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);//将缓冲区左移，将未使用的位设为0
        fwrite(&buffer, sizeof(unsigned char), 1, fout);//写入未使用的位到目标文件
    }
    
    fclose(fin);
    fclose(fout);
    printf(">>> 编码完成! 请在%s中查看\n", dst_file);
}


// 计算熵
double CalculateEntropy(HuffmanTree HT, int n, long total_chars) {
    double entropy = 0.0;//用于存储熵值
    for (int i = 1; i <= n; i++) {
        int freq = HT[i].weight;//直接使用Huffman树中的权重作为频率
        if (freq > 0) {
            double p = (double)freq / (double)total_chars;//计算字符概率
            entropy -= p * log2(p);//计算熵值
        }
    }
    return entropy;
}

// 计算平均码长 
double CalculateAverageCodeLength(HuffmanTree HT, HuffmanCode HC, int n, long total_chars) {
    double avg_length = 0.0;
    for (int i = 1; i <= n; i++) {
        int freq = HT[i].weight;//直接使用Huffman树中的权重作为频率
        if (freq > 0) {
            double p = (double)freq / total_chars;//计算字符概率
            int code_length = strlen(HC.HCPointer[i]);//计算字符对应二进制编码长度
            avg_length += p * code_length;//计算平均码长
        }
    }
    return avg_length;//返回平均码长
}

// 计算压缩率
void CalculateCompressionRatio(const char *src_file, const char *encoded_file) {
    FILE *fp1 = fopen(src_file, "rb");//打开源文件
    FILE *fp2 = fopen(encoded_file, "rb");//打开压缩后的文件
    
    if (!fp1 || !fp2) {
        printf("错误: 无法计算压缩率。\n");
        return;
    }
    
    fseek(fp1, 0, SEEK_END);//fseek用于定位文件指针到文件末尾，用于计算文件大小
    long src_size = ftell(fp1);//ftell用于获取文件指针当前位置，即文件大小
    fseek(fp2, 0, SEEK_END);//fseek用于定位文件指针到文件末尾，用于计算文件大小
    long encoded_size = ftell(fp2);//ftell用于获取文件指针当前位置，即文件大小
    
    fclose(fp1);
    fclose(fp2);
    
    if (src_size == 0) {
        printf("错误: 源文件为空。\n");
        return;
    }
    
    double ratio = (double)encoded_size / src_size * 100.0;
    printf("\n=== 压缩率分析 ===\n");
    printf("原文件大小: %ld 字节\n", src_size);
    printf("压缩后文件大小: %ld 字节\n", encoded_size);
    printf("压缩率: %.2f%%\n", ratio);
}

void ShowInformationTheoryMetrics(HuffmanTree HT, HuffmanCode HC, int n, long total_chars) {
    if (!HC.HCPointer || n <= 0) {
        printf("请先初始化编码。\n");
        return;
    }
    
    double H = CalculateEntropy(HT, n, total_chars);
    double L = CalculateAverageCodeLength(HT, HC, n, total_chars);
    double efficiency = (H / L) * 100.0;
    
    printf("\n=== 信息论定量分析 ===\n");
    printf("信源熵 H(X): %.4f bits/符号\n", H);
    printf("平均码长 L: %.4f bits/符号\n", L);
    printf("编码效率 η: %.2f%%\n", efficiency);
    
    CalculateCompressionRatio("source.txt", "encoded.txt");
}

bool VerifyFileIntegrity(const char *file1, const char *file2) {
    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");
    
    if (!fp1 || !fp2) {
        printf("错误: 无法打开文件进行校验。\n");
        return false;
    }
    
    bool is_identical = true;
    int ch1, ch2;
    long position = 0;
    
    while ((ch1 = fgetc(fp1)) != EOF && (ch2 = fgetc(fp2)) != EOF) {
        if (ch1 != ch2) {
            is_identical = false;
            printf("差异发现于位置 %ld: 文件1=0x%02X, 文件2=0x%02X\n", position, ch1, ch2);
            break;
        }
        position++;
    }
    
    if (is_identical && ((ch1 = fgetc(fp1)) != EOF || (ch2 = fgetc(fp2)) != EOF)) {
        is_identical = false;
        printf("文件长度不一致。\n");
    }
    
    fclose(fp1);
    fclose(fp2);
    
    return is_identical;
}
