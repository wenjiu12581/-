#include <stdio.h>
#include <stdlib.h>

// (5,2)线性分组码参数
#define N 5  // 码长
#define K 2  // 信息位
#define R 3  // 监督位

// 生成矩阵 G (系统码形式)
int G[K][N] = {
    {1, 0, 1, 0, 1},
    {0, 1, 0, 1, 1}
};//系统码，前面是I_k，后面是P

// 监督矩阵 H (由 G = [I_k | P] 可得 H = [-P^T | I_r])
int H[R][N] = {
    {1, 0, 1, 0, 0},
    {0, 1, 0, 1, 0},
    {1, 1, 0, 0, 1}
};


// 编码函数：输入2位信息，输出5位码字
void encode(int *message, int *codeword) {
    for (int i = 0; i < N; i++) {
        codeword[i] = 0;//初始化码字为0，因为0异或任何数都等于任何数
        for (int j = 0; j < K; j++) {
            codeword[i] ^= (message[j] & G[j][i]);//信息位的第j位与矩阵G的第j行第i列按位异或，得到码字的第i位
            }
    }
}//对于真正要串数的2位信息位，矩阵乘G，得到5位码字

// 计算伴随式，输入5位码字，输出3位伴随式
void compute_syndrome(int *received, int *syndrome) {
    for (int i = 0; i < R; i++) {
        syndrome[i] = 0;//初始化伴随式为0，因为0异或任何数都等于任何数
        for (int j = 0; j < N; j++) {
            syndrome[i] ^= (received[j] & H[i][j]);//实质是拿5位码字矩阵乘H的转置，最终得到的伴随式
        }
    }
}

// 打印二进制数组
void print_binary(int *arr, int len) {
    for (int i = 0; i < len; i++) {
        printf("%d", arr[i]);
    }
}

// 标准阵列译码表结构
typedef struct {
    int syndrome[R];      // 伴随式
    int coset_leader[N];  // 陪集首（错误图样）
} CosetTable;

// 初始化标准阵列
void init_coset_table(CosetTable *table) {
    // 生成所有2^R=8种伴随式及其对应的陪集首
    // 陪集首按重量从小到大排列（标准阵列第一列）
    
    // 重量0（全零错误）
    int syndrome0[R] = {0, 0, 0};
    int leader0[N] = {0, 0, 0, 0, 0};
    for (int i = 0; i < R; i++) table[0].syndrome[i] = syndrome0[i];
    for (int i = 0; i < N; i++) table[0].coset_leader[i] = leader0[i];//当只有0位错误时，伴随式是000，陪集首是00000
    
    // 重量1的错误
    // 根据 H计算：S =  H*e^T（3,5*5,1)
    int syndromes1[5][R] = {
        {1, 0, 1},  // 错误在第0位 e=[1,0,0,0,0]
        {0, 1, 1},  // 错误在第1位 e=[0,1,0,0,0]
        {1, 0, 0},  // 错误在第2位 e=[0,0,1,0,0]
        {0, 1, 0},  // 错误在第3位 e=[0,0,0,1,0]
        {0, 0, 1}   // 错误在第4位 e=[0,0,0,0,1]
    };
    int leaders1[5][N] = {
        {1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1}
    };
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < R; j++) table[i+1].syndrome[j] = syndromes1[i][j];
        for (int j = 0; j < N; j++) table[i+1].coset_leader[j] = leaders1[i][j];
    }//截至这里8种伴随式已经确定了6种，且每一个伴随式都找到了对应陪集首
    
    // 重量2的错误（剩余2种伴随式）
    // S=[1,1,0] -> 错误e=[0,0,1,1,0] (第3和第4位)
    // S=[1,1,1] -> 错误e=[0,1,1,0,0] (第1和第2位)
    //这里的e是根据s倒推出来的
    int syndromes2[2][R] = {
        {1, 1, 0},  // S=[1,0,1]
        {1, 1, 1}   // S=[1,1,1]
    };
    int leaders2[2][N] = {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 0, 0}
    };
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < R; j++) table[i+6].syndrome[j] = syndromes2[i][j];
        for (int j = 0; j < N; j++) table[i+6].coset_leader[j] = leaders2[i][j];
    }//截至这里8种伴随式已经确定了8种，且每一个伴随式都找到了对应陪集首
}

// 查找伴随式对应的陪集首
int find_coset_leader(CosetTable *table, int *syndrome, int *coset_leader) {
    // 遍历8种可能的伴随式
    for (int i = 0; i < (1 << R); i++) {
        int match = 1;
        for (int j = 0; j < R; j++) {
            if (table[i].syndrome[j] != syndrome[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            for (int j = 0; j < N; j++) {
                coset_leader[j] = table[i].coset_leader[j];
            }
            return 1;
        }
    }
    return 0;
}//将刚计算出的伴随式s与先前写好的8种伴随式比较，如果匹配，就返回对应的陪集首，否则返回0

// 译码函数
void decode(int *received, int *decoded, CosetTable *table) {
    int syndrome[R];
    int coset_leader[N];
    
    // 计算伴随式
    compute_syndrome(received, syndrome);
    
    // 查找陪集首
    find_coset_leader(table, syndrome, coset_leader);
    
    // 译码：c = r + e (模2加)
    for (int i = 0; i < N; i++) {
        decoded[i] = (received[i] ^ coset_leader[i]) ;
    }
}

// 从码字提取信息位
void extract_message(int *codeword, int *message) {
    // 系统码，前K位是信息位
    for (int i = 0; i < K; i++) {
        message[i] = codeword[i];
    }
}

int main() {
    // 初始化标准阵列
    CosetTable coset_table[1 << R];//8种伴随式
    init_coset_table(coset_table);
    
    printf("========== (5,2)线性分组码实验 ==========\n");
    printf("生成矩阵G:\n");
    for (int i = 0; i < K; i++) {
        printf("  ");
        print_binary(G[i], N);
        printf("\n");
    }
    
    printf("\n监督矩阵H:\n");
    for (int i = 0; i < R; i++) {
        printf("  ");
        print_binary(H[i], N);
        printf("\n");
    }
    
    // 生成所有4种可能的信息及其码字
    printf("\n所有信息码字映射:\n");
    for (int i = 0; i < (1 << K); i++) {
        int message[K] = {(i >> 1) & 1, i & 1};//生成所有4种可能的信息，是（1，2）的
        int codeword[N];//生成所有4种可能的码字，是（1，5）的
        encode(message, codeword);//根据信息生成码字
        
        printf("  信息 ");
        print_binary(message, K);
        printf(" -> 码字 ");
        print_binary(codeword, N);
        printf("\n");
    }
    
    // 用户输入译码
    printf("\n========== 用户输入译码 ==========\n");
    
    while (1) {
        int received[N];
        int decoded[N];
        int message[K];
        
        printf("\n请输入5位接收码字（用空格分隔，如：1 0 1 1 0），输入-1退出：\n");
        
        // 读取用户输入
        int valid_input = 1;
        for (int i = 0; i < N; i++) {
            if (scanf("%d", &received[i]) != 1) {
                valid_input = 0;
                break;
            }
            // 如果输入-1，退出程序
            if (received[i] == -1) {
                printf("程序已退出。\n");
                return 0;
            }
            // 确保是二进制（0或1）
            received[i] = received[i] % 2;
            if (received[i] < 0) received[i] = 0;
        }
        
        if (!valid_input) {
            printf("输入错误，请重新输入。\n");
            // 清除输入缓冲区
            while (getchar() != '\n');
            continue;
        }
        
        // 进行译码
        decode(received, decoded, coset_table);
        extract_message(decoded, message);
        
        // 输出结果
        printf("\n  接收码字: ");
        print_binary(received, N);
        printf("\n  译码结果: ");
        print_binary(decoded, N);
        printf("\n  提取信息: ");
        print_binary(message, K);
        printf("\n");
    }
    
    return 0;
}