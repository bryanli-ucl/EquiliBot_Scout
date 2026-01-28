#!/usr/bin/env python3
# hex_6x8_generator.py
# 生成 6x8 点阵的 16 进制数字

def create_6x8_digits():
    """
    定义所有 16 进制数字的 6x8 点阵
    每行用6位二进制表示
    """
    digits = {
        0x0: [  # 0
            0b011110,
            0b100001,
            0b100011,
            0b100101,
            0b101001,
            0b110001,
            0b100001,
            0b011110
        ],
        0x1: [  # 1
            0b001000,
            0b011000,
            0b001000,
            0b001000,
            0b001000,
            0b001000,
            0b001000,
            0b011100
        ],
        0x2: [  # 2
            0b011110,
            0b100001,
            0b000001,
            0b000010,
            0b000100,
            0b001000,
            0b010000,
            0b111111
        ],
        0x3: [  # 3
            0b011110,
            0b100001,
            0b000001,
            0b000110,
            0b000001,
            0b000001,
            0b100001,
            0b011110
        ],
        0x4: [  # 4
            0b000010,
            0b000110,
            0b001010,
            0b010010,
            0b100010,
            0b111111,
            0b000010,
            0b000010
        ],
        0x5: [  # 5
            0b111111,
            0b100000,
            0b100000,
            0b111110,
            0b000001,
            0b000001,
            0b100001,
            0b011110
        ],
        0x6: [  # 6
            0b001110,
            0b010000,
            0b100000,
            0b111110,
            0b100001,
            0b100001,
            0b100001,
            0b011110
        ],
        0x7: [  # 7
            0b111111,
            0b000001,
            0b000010,
            0b000100,
            0b001000,
            0b010000,
            0b010000,
            0b010000
        ],
        0x8: [  # 8
            0b011110,
            0b100001,
            0b100001,
            0b011110,
            0b100001,
            0b100001,
            0b100001,
            0b011110
        ],
        0x9: [  # 9
            0b011110,
            0b100001,
            0b100001,
            0b100001,
            0b011111,
            0b000001,
            0b000010,
            0b011100
        ],
        0xA: [  # A
            0b001100,
            0b010010,
            0b100001,
            0b100001,
            0b111111,
            0b100001,
            0b100001,
            0b100001
        ],
        0xB: [  # B
            0b111110,
            0b100001,
            0b100001,
            0b111110,
            0b100001,
            0b100001,
            0b100001,
            0b111110
        ],
        0xC: [  # C
            0b011110,
            0b100001,
            0b100000,
            0b100000,
            0b100000,
            0b100000,
            0b100001,
            0b011110
        ],
        0xD: [  # D
            0b111100,
            0b100010,
            0b100001,
            0b100001,
            0b100001,
            0b100001,
            0b100010,
            0b111100
        ],
        0xE: [  # E
            0b111111,
            0b100000,
            0b100000,
            0b111110,
            0b100000,
            0b100000,
            0b100000,
            0b111111
        ],
        0xF: [  # F
            0b111111,
            0b100000,
            0b100000,
            0b111110,
            0b100000,
            0b100000,
            0b100000,
            0b100000
        ]
    }
    return digits

def print_digit_preview(digit_data, label):
    """
    打印数字预览
    """
    print(f"\n{label}:")
    for row in digit_data:
        binary_str = f"{row:06b}"
        visual = binary_str.replace('0', '·').replace('1', '█')
        print(f"  {visual}  (0b{binary_str})")

def generate_arduino_array():
    """
    生成 Arduino 使用的 C++ 数组
    """
    digits = create_6x8_digits()
    
    code = "// 6x8 点阵字体 - 16进制数字 0-F\n"
    code += "// 每个数字占 6 列宽，8 行高\n\n"
    code += "const uint8_t font_6x8[16][8] = {\n"
    
    for i in range(16):
        code += f"  // 0x{i:X}\n"
        code += "  {"
        rows = []
        for row in digits[i]:
            rows.append(f"0b{row:06b}")
        code += ", ".join(rows)
        code += "}"
        if i < 15:
            code += ","
        code += "\n"
    
    code += "};\n"
    return code

def generate_hex_lookup():
    """
    生成十六进制值的查找代码
    """
    code = "\n// 使用示例：显示单个16进制数字\n"
    code += "void displayHexDigit(uint8_t digit, int startCol) {\n"
    code += "  // digit: 0-15 (0x0-0xF)\n"
    code += "  // startCol: 起始列位置 (0-11)\n"
    code += "  uint8_t frame[8][12] = {0};\n"
    code += "  \n"
    code += "  for(int row = 0; row < 8; row++) {\n"
    code += "    uint8_t pattern = font_6x8[digit][row];\n"
    code += "    for(int col = 0; col < 6; col++) {\n"
    code += "      if(startCol + col < 12) {\n"
    code += "        frame[row][startCol + col] = (pattern >> (5 - col)) & 0x01;\n"
    code += "      }\n"
    code += "    }\n"
    code += "  }\n"
    code += "  \n"
    code += "  // 转换并显示\n"
    code += "  uint32_t frameData[3];\n"
    code += "  bitmapToFrame(frame, frameData);\n"
    code += "  matrix.loadFrame(frameData);\n"
    code += "}\n"
    return code

def main():
    print("=" * 60)
    print("6x8 点阵字体生成器 - 16进制数字 0-F")
    print("=" * 60)
    
    digits = create_6x8_digits()
    
    # 打印所有数字的预览
    print("\n【字体预览】\n")
    for i in range(16):
        print_digit_preview(digits[i], f"0x{i:X}")
    
    # 生成 Arduino 代码
    arduino_code = generate_arduino_array()
    arduino_code += generate_hex_lookup()
    
    print("\n" + "=" * 60)
    print("【Arduino 代码】")
    print("=" * 60)
    print(arduino_code)
    
    # 保存到文件
    with open("font_6x8.h", "w", encoding="utf-8") as f:
        f.write(arduino_code)
    
    print("\n✓ 代码已保存到 font_6x8.h")
    
    # 生成完整示例
    generate_complete_example()

def generate_complete_example():
    """
    生成完整的Arduino示例程序
    """
    example = """
// 完整示例：显示两个16进制数字
void displayTwoHexDigits(uint8_t value) {
  uint8_t highNibble = (value >> 4) & 0x0F;  // 高4位
  uint8_t lowNibble = value & 0x0F;           // 低4位
  
  uint8_t frame[8][12] = {0};
  
  // 第一个数字 (列 0-5)
  for(int row = 0; row < 8; row++) {
    uint8_t pattern = font_6x8[highNibble][row];
    for(int col = 0; col < 6; col++) {
      frame[row][col] = (pattern >> (5 - col)) & 0x01;
    }
  }
  
  // 第二个数字 (列 6-11)
  for(int row = 0; row < 8; row++) {
    uint8_t pattern = font_6x8[lowNibble][row];
    for(int col = 0; col < 6; col++) {
      frame[row][col + 6] = (pattern >> (5 - col)) & 0x01;
    }
  }
  
  // 转换并显示
  uint32_t frameData[3];
  bitmapToFrame(frame, frameData);
  matrix.loadFrame(frameData);
}
"""
    
    with open("example_usage.txt", "w", encoding="utf-8") as f:
        f.write(example)
    
    print("✓ 使用示例已保存到 example_usage.txt")

if __name__ == "__main__":
    main()