## 用法

### 命令行传参

#### 参数说明

| 参数             | 说明                                      |
| ---------------- | ----------------------------------------- |
| `[文件列表]`     | 输入文件列表（可直接指定，无需`-i`前缀）  |
| `-i, --input`    | 输入文件或目录（可与其他文件混合使用）    |
| `-r, --rows`     | 行数（0表示自动计算）                     |
| `-c, --cols`     | 列数（0表示自动计算）                     |
| `-m, --margin`   | 图片间距（默认10像素）                    |
| `-o, --output`   | 输出文件路径（默认`stitched_image.png` ） |
| `-s, --sequence` | 在图像左上角添加序号                      |
| `-d, --datetime` | 在图像上添加文件修改时间                  |
| `-M, --mosaic`   | 对检测到的文本框区域添加马赛克            |
| `-h, --help`     | 显示帮助信息                              |

#### 使用方法示例

1. **基本用法（混合使用各种输入方式）**：

   ```Bash
   # 直接指定文件（无需-i前缀）
   ./ImgStitcher.exe  "image1.jpg"  "image2.png"  "image3.jpeg" 
   
   # 混合使用-i参数和直接文件参数 
   ./ImgStitcher.exe  -i dir1 "image1.jpg"  -i "image2.png"  "image3.jpeg"  -o result.jpg  
   ```

2. **自动布局模式**：

   ```Bash
   # 自动计算最佳行列布局 
   ./ImgStitcher.exe  "images/*.png"  # 支持通配符 
   ```

3. **高级处理选项**：

   ```Bash
   # 3行4列布局，20像素间距，添加序号/时间/马赛克 
   ./ImgStitcher.exe  -i "screenshots/" -r 3 -c 4 -m 20 -s -d -M -o "result.png" 
   ```

4. **特殊用法**：

   ```Bash
   # 处理不同目录的文件 
   ./ImgStitcher.exe  "dir1/*.jpg" "dir2/*.png" -o combined.jpg  
   
   # 使用通配符选择文件 
   ./ImgStitcher.exe  "images/2024*.png" -o output.jpg  
   ```

5. **查看帮助**：

   ```Bash
   ./ImgStitcher.exe  -h 
   ```

#### 注意事项

1. 输入文件支持：
   - 直接文件路径（支持相对/绝对路径）
   - 目录路径（自动包含目录下所有图片）
   - 通配符（如 `*.png`）
2. 参数顺序：
   - 选项参数（如`-o`）可以任意位置
   - 输入文件参数应该连续放置
3. 自动计算规则：
   - 当`--rows`或`--cols`为0时自动计算
   - 计算公式：`行数 = ceil(sqrt(图片总数))`，`列数 = ceil(图片总数/行数)`
4. 文件格式支持：
   - 支持读取：JPEG、PNG
   - 支持输出：PNG（默认）、JPEG（需注意OpenCV可能的问题）



### GUI

直接双击打开