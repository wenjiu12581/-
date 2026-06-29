#include <stdio.h>
#include <string.h>

#define MAXN 256

// 打印比特数组，格式：[1, 0, 1, 1]
void print_bits(const int a[], int len) {
    printf("[");
    for (int i = 0; i < len; i++) {
        printf("%d", a[i]);
        if (i != len - 1) printf(", ");
    }
    printf("]");
}

// GF(2) 多项式除法：
// dividend 是被除多项式，divisor 是除式
// remainder 返回余数，长度为 divisor_len - 1
void gf2_divide(const int dividend[], int dividend_len,
                const int divisor[], int divisor_len,
                int remainder[]) {
    int temp[MAXN];
    memset(temp, 0, sizeof(temp));

    for (int i = 0; i < dividend_len; i++) {
        temp[i] = dividend[i];
    }// 初始化 temp 为 dividend 的副本

    // 模2长除法：遇到 1 就异或一次除式
    for (int i = 0; i <= dividend_len - divisor_len; i++) {
        if (temp[i] == 1) {
            for (int j = 0; j < divisor_len; j++) {
                temp[i + j] ^= divisor[j];// 异或除式
                           }
        }
    }

    // 最后 divisor_len-1 位就是余数
    int rem_len = divisor_len - 1;
    int start = dividend_len - rem_len;
    for (int i = 0; i < rem_len; i++) {
        remainder[i] = temp[start + i];
    }
}

// 系统循环码编码
// msg: 信息位，长度 k
// g: 生成多项式，长度 r+1
// codeword: 输出码字，长度 n = k + r
void encode_systematic(const int msg[], int k,
                       const int g[], int g_len,
                       int codeword[]) {
    int r = g_len - 1;
    int dividend[MAXN];
    int remainder[MAXN];

    // 1) 信息位后补 r 个 0
    for (int i = 0; i < k; i++) {
        dividend[i] = msg[i];
    }
    for (int i = k; i < k + r; i++) {
        dividend[i] = 0;
    }

    // 2) 计算余数
    gf2_divide(dividend, k + r, g, g_len, remainder);

    // 3) 系统码字 = 原信息位 + 余数
    for (int i = 0; i < k; i++) {
        codeword[i] = msg[i];
    }
    for (int i = 0; i < r; i++) {
        codeword[k + i] = remainder[i];
    }
}

// 循环右移 steps 位
void rotate_right(const int src[], int n, int steps, int dst[]) {
    steps %= n;
    for (int i = 0; i < n; i++) {
        dst[(i + steps) % n] = src[i];
    }
}

// 判断余数是否全 0
int is_zero_remainder(const int rem[], int len) {
    for (int i = 0; i < len; i++) {
        if (rem[i] != 0) return 0;
    }
    return 1;
}

int main() {
    // 按实验 PDF 的参数固定：
    // n = 7, k = 4, r = 3
    // g(x) = x^3 + x + 1 => [1, 0, 1, 1]
    int n = 7;
    int k = 4;
    int g[] = {1, 0, 1, 1};
    int g_len = 4;
    int r = g_len - 1;

    int msg[4];
    int codeword[7];
    int rotated[7];
    int rem1[3];
    int rem2[3];
    int dividend[7];

    printf("========== 循环码编码实验 ==========\n");
    printf("码长 n = %d\n", n);
    printf("信息位 k = %d\n", k);
    printf("监督位 r = %d\n", r);
    printf("生成多项式 g(x) = ");
    print_bits(g, g_len);
    printf("\n");

    printf("请输入 4 位信息位（用空格分开，例如 1 0 1 1）：\n");
    for (int i = 0; i < k; i++) {
        scanf("%d", &msg[i]);
    }

    // 编码
    encode_systematic(msg, k, g, g_len, codeword);

    printf("原始信息位 = ");
    print_bits(msg, k);
    printf("\n");

    printf("编码得到的循环码码字 = ");
    print_bits(codeword, n);
    printf("\n");

    // 验证码字能否被 g(x) 整除
    gf2_divide(codeword, n, g, g_len, rem1);
    printf("该码字是否能被 g(x) 整除： %s\n",
           is_zero_remainder(rem1, r) ? "True" : "False");

    // 做一次循环右移
    rotate_right(codeword, n, 1, rotated);
    printf("循环右移 1 位后的码字 = ");
    print_bits(rotated, n);
    printf("\n");

    // 验证循环移位后是否仍是合法码字
    gf2_divide(rotated, n, g, g_len, rem2);
    printf("循环移位后是否仍为合法码字： %s\n",
           is_zero_remainder(rem2, r) ? "True" : "False");

    return 0;
}