## 用法

### 命令行传参

| 参数           | 说明                                    |
| -------------- | --------------------------------------- |
| -i, --input    | 输入文件或目录（可指定多次）            |
| -r, --rows     | 行数（0表示自动计算）                   |
| -c, --cols     | 列数（0表示自动计算）                   |
| -m, --margin   | 图片间距（默认10）                      |
| -o, --output   | 输出文件路径（默认stitched_image.png ） |
| -s, --sequence | 添加序号                                |
| -d, --datetime | 添加日期时间                            |
| -M, --mosaic   | 添加马赛克                              |
| -h, --help     | 显示帮助信息                            |

#### 使用方法示例：

1. **基本用法**：

   ```Bash
   ./ImgStitcher.exe -i dir1 -i file1.jpg  -i file2.jpg  -o output.png  
   ```

2. **自动计算行列数**：

   ```Bash
   ./ImgStitcher.exe -i images_dir  # 自动计算行列数 
   ```

3. **指定处理选项**：

   ```Bash
   ./ImgStitcher.exe -i dir -r 3 -c 4 -m 20 -s -d -M -o result.png  
   ```

4. **查看帮助**：

   ```Bash
   ./ImgStitcher.exe -h 
   ```



### GUI

直接双击打开