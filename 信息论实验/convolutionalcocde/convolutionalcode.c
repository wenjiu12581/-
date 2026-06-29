#include <stdio.h>
#include <string.h>

// 异或函数（模2加法）
int xor_bits(int a, int b)
{
    return a ^ b;
}

int main()
{
    // 输入信息序列
    char input[] = "10110";

    int len = strlen(input);

    // 移位寄存器
    int reg[3] = {0, 0, 0};

    // 保存编码结果
    char output[100] = "";

    int i;

    printf("========== 卷积码编码实验 ==========\n");
    printf("信息序列： %s\n", input);

    printf("生成多项式：\n");
    printf("g1 = 111\n");
    printf("g2 = 101\n\n");

    printf("===== 编码过程 =====\n\n");

    // =========================
    // 正常输入部分
    // =========================
    for(i = 0; i < len; i++)
    {
        // 当前输入位
        int u = input[i] - '0';

        // 寄存器移位
        reg[2] = reg[1];
        reg[1] = reg[0];
        reg[0] = u;

        // 根据生成多项式计算输出
        // g1 = 111
        int v1 = reg[0] ^ reg[1] ^ reg[2];

        // g2 = 101
        int v2 = reg[0] ^ reg[2];

        // 输出当前状态
        printf("输入: %d\n", u);
        printf("寄存器状态: [%d %d %d]\n",
               reg[0], reg[1], reg[2]);

        printf("v1 = %dxor%dxor%d = %d\n",
               reg[0], reg[1], reg[2], v1);

        printf("v2 = %dxor%d = %d\n",
               reg[0], reg[2], v2);

        printf("输出: %d%d\n\n", v1, v2);

        // 保存输出
        int out_len = strlen(output);

        output[out_len] = v1 + '0';
        output[out_len + 1] = v2 + '0';
        output[out_len + 2] = '\0';
    }

    // 保存不加终止比特结果
    char output_no_tail[100];
    strcpy(output_no_tail, output);

    // =========================
    // 添加终止比特
    // =========================

    printf("===== 添加终止比特 =====\n\n");

    // m=3，所以添加 m-1 = 2 个0
    for(i = 0; i < 2; i++)
    {
        int u = 0;

        // 寄存器移位
        reg[2] = reg[1];
        reg[1] = reg[0];
        reg[0] = u;

        // 计算输出
        int v1 = reg[0] ^ reg[1] ^ reg[2];
        int v2 = reg[0] ^ reg[2];

        printf("输入终止比特: %d\n", u);
        printf("寄存器状态: [%d %d %d]\n",
               reg[0], reg[1], reg[2]);

        printf("输出: %d%d\n\n", v1, v2);

        // 保存输出
        int out_len = strlen(output);

        output[out_len] = v1 + '0';
        output[out_len + 1] = v2 + '0';
        output[out_len + 2] = '\0';
    }

    // =========================
    // 输出最终结果
    // =========================

    printf("===== 最终结果 =====\n\n");

    printf("不添加终止比特的编码结果：\n");
    printf("%s\n\n", output_no_tail);

    printf("添加终止比特后的编码结果：\n");
    printf("%s\n", output);

    return 0;
}