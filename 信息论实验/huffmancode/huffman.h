#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INFINITY_INT 10000 // 无穷大值 
#define OK 1
#define ERROR 0

typedef int Status;

typedef struct {
    int weight;                 // 权值
    int parent, lchild, rchild; // 父节点，左孩子，右孩子
} HTNode, *HuffmanTree;        //顺序存储 


typedef struct {
    char* chars;      // 字符数组，存储实际的字符
    char** HCPointer; // 字符指针数组 (存储对应的01编码串)
} HuffmanCode;


void Select(HuffmanTree HT, int end, int &s1, int &s2);
//在当前的huffman树中找到节点为空且权值最小的两个节点，返回节点位置 

void CreateHuffmanTree(HuffmanTree &HT, int *weights, int n);
//创建一个Huffman树，记录权值 
 
void GenerateHuffmanCodes(HuffmanTree HT, HuffmanCode &HC, char *chars, int n);
// 为每个节点生成Huffman编码 


Status DecodeFile(HuffmanTree &HT, HuffmanCode &HC, const char *encoded_filename, const char *decoded_filename);
//将01编码串解码回原始的文本 

void FreeHuffman(HuffmanTree &HT, HuffmanCode &HC, int n);
//释放Huffman树和编码的内存
 
#endif
