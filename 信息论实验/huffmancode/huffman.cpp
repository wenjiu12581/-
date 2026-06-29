#include "huffman.h"


void Select(HuffmanTree HT, int end, int &s1, int &s2) {
    int min1 = INFINITY_INT; 
    int min2 = INFINITY_INT; 
	//比较得到最小值，先假设为无穷大，然后每次取较小值作为最小值，找到次小值 
    s1 = 0;
    s2 = 0;

    for (int i = 1; i <= end; i++) {
        //只找节点为空的节点 
        if (HT[i].parent == 0) {
            if (HT[i].weight < min1) {
                min2 = min1;
                s2 = s1;
                min1 = HT[i].weight;
                s1 = i;
            }//如果找到一个比当前最小值min1还小的值，那么记录它作为最小值，并将原来的最小值变为次小值 
           
            else if (HT[i].weight < min2) {
                min2 = HT[i].weight;
                s2 = i;
            }//如果该值比次小值小但比最小值大，那么只需要更新次小值 
        }
    }
}//在当前范围内找到节点为空且权值最小的两个节点 

//功能: 构建Huffman树 
void CreateHuffmanTree(HuffmanTree &HT, int *weights, int n) {
    if (n <= 1) return;
    
    int m = 2 * n - 1; //原始有n个节点，每次合并减少一个节点，共需要2n-1个节点 
    
    HT = (HuffmanTree)malloc((m + 1) * sizeof(HTNode));//0号位置不用 

    for (int i = 1; i <= n; ++i) {
        HT[i].weight = weights[i - 1]; 
        HT[i].parent = 0;
        HT[i].lchild = 0;
        HT[i].rchild = 0;
    }//将原始的n个节点写入树中 
    

    for (int i = n + 1; i <= m; ++i) {
        HT[i].weight = 0;
        HT[i].parent = 0;
        HT[i].lchild = 0;
        HT[i].rchild = 0;
    }//将剩余的节点写入，权值暂时为0 

    
    int s1, s2;
    for (int i = n + 1; i <= m; ++i) {
        Select(HT, i - 1, s1, s2); //以i-1作为end，也就是说每次只处理1到i-1的节点 
		//找到权值最小的两个节点，将它们的权值相加作为新节点的权值，并将新节点作为它们的父节点 
        HT[s1].parent = i;
        HT[s2].parent = i;
        HT[i].lchild = s1;
        HT[i].rchild = s2;
        HT[i].weight = HT[s1].weight + HT[s2].weight; 
    }
}//成功构建Huffman树 



//功能: 从叶子节点开始，为每个字符生成Huffman编码 
void GenerateHuffmanCodes(HuffmanTree HT, HuffmanCode &HC, char *chars, int n) {
    
    HC.chars = (char *)malloc((n + 1) * sizeof(char));
	//只有原始的n个节点需要huffman编码，0号位置不用，所以分配n+1的空间 
    //分配空间来存储实际的字符
    HC.HCPointer = (char **)malloc((n + 1) * sizeof(char*));
    //分配空间来存储每个字符对应的Huffman编码，每个编码都是由01组成的，所以用二级指针 
    
     
    // 临时空间，用来存储当前生成的编码串 
    char *cd = (char *)malloc(n * sizeof(char));
    cd[n - 1] = '\0'; // 结束符

    for (int i = 1; i <= n; ++i) {
        HC.chars[i] = chars[i - 1]; // 存储字符信息
        int start = n - 1;          // start指向编码的起始位置
        int c = i;                  // c为当前节点
        int f = HT[i].parent;       // f为父节点

         
        while (f != 0) {
            if (HT[f].lchild == c) {
                cd[--start] = '0'; // 左子树为 '0'
            } else {
                cd[--start] = '1'; // 右子树为 '1' 
            }
            c = f;
            f = HT[f].parent;// 从叶子节点向上，直到根节点 
        }//此时cd中存储的就是chars[i-1]对应的huffman编码 

        // 为第i个字符分配相应的空间，去掉多余部分
        HC.HCPointer[i] = (char *)malloc((n - start) * sizeof(char));
        strcpy(HC.HCPointer[i], &cd[start]);
    }

    free(cd); // 释放临时空间 
}

 
 //功能: 读取编码文件，根据Huffman编码规则解码，将结果写入目标文件
Status DecodeFile(HuffmanTree &HT, HuffmanCode &HC, const char *encoded_filename, const char *decoded_filename) {
    FILE *fin = fopen(encoded_filename, "rb");
    FILE *fout = fopen(decoded_filename, "w");
    
    if (!fin || !fout) {
        printf("错误: 无法打开文件进行解码。\n");
        return ERROR;
    }

    int n;
    fread(&n, sizeof(int), 1, fin);
    
    char *chars = (char *)malloc(n * sizeof(char));
    int *weights = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        fread(&chars[i], sizeof(char), 1, fin);
        fread(&weights[i], sizeof(int), 1, fin);
    }
    
    CreateHuffmanTree(HT, weights, n);
    GenerateHuffmanCodes(HT, HC, chars, n);
    
    int m = 2 * n - 1;
    int p = m;
    unsigned char byte;
    int bit_position;
    
    while (fread(&byte, sizeof(unsigned char), 1, fin) == 1) {
        bit_position = 8;
        while (bit_position > 0) {
            bit_position--;
            int bit = (byte >> bit_position) & 1;
            
            if (bit == 0) {
                p = HT[p].lchild;
            } else {
                p = HT[p].rchild;
            }

            if (HT[p].lchild == 0 && HT[p].rchild == 0) {
                if (p <= n) {
                    fputc(chars[p - 1], fout);
                }
                p = m;
            }
        }
    }

    fclose(fin);
    fclose(fout);
    
    free(chars);
    free(weights);
    
    return OK;
}

void FreeHuffman(HuffmanTree &HT, HuffmanCode &HC, int n) {
    if (HT) free(HT);
    if (HC.HCPointer) {
        for(int i = 1; i <= n; i++) {
            if(HC.HCPointer[i]) free(HC.HCPointer[i]);
        }
        free(HC.HCPointer);
    }
    if (HC.chars) free(HC.chars);
}
