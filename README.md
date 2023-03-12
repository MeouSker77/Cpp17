# C++17完全指南

本书为《C++17 the complete guide》(第一版) by Nicolai M. Josuttis的个人中文翻译版，
仅供学习和交流使用，如有侵权请联系作者删除

## 以下为英文原版购买地址

* 电子版[https://leanpub.com/cpp17](https://leanpub.com/cpp17)
* 印刷版[https://t.co/aJQbbvmt4Z](https://t.co/aJQbbvmt4Z)

---

## 以下为中文翻译版

### pdf 版（推荐）
[点此下载](https://github.com/MeouSker77/Cpp17/releases/download/v1.0/Cpp17.pdf)

注:
- 在github中点击上方链接可能会显示“something went wrong”，可以下载之后在本地打开。
- 貌似latex默认并不会嵌入用到的字体，如果pdf打开之后某一种字体看起来怪怪的，那大概率是缺少对应的字体，本书用到的字体有宋体、楷体、Times New Roman、Consolas

### markdown 版目录
- 第一部分 基本语言特性
  - [第一章 结构化绑定](markdown/src/ch01.md)
  - [第二章 带初始化的 if 和 switch 语句](markdown/src/ch02.md)
  - [第三章 内联变量](markdown/src/ch03.md)
  - [第四章 聚合体扩展](markdown/src/ch04.md)
  - [第五章 强制省略拷贝或传递未实质化的对象](markdown/src/ch05.md)
  - [第六章 lambda 表达式扩展](markdown/src/ch06.md)
  - [第七章 新属性和属性特性](markdown/src/ch07.md)
  - [第八章 其他语言特性](markdown/src/ch08.md)
- 第二部分 模板特性
  - [第九章 类模板参数推导](markdown/src/ch09.md)
  - [第十章 编译期 if 语句](markdown/src/ch10.md)
  - [第十一章 折叠表达式](markdown/src/ch11.md)
  - [第十二章 处理字符串字面量模板参数](markdown/src/ch12.md)
  - [第十三章 占位符类型作为模板参数](markdown/src/ch13.md)
  - [第十四章 扩展的 using 声明](markdown/src/ch14.md)
- 第三部分 新的标准库组件
  - [第十五章 std::optional<>](markdown/src/ch15.md)
  - [第十六章 std::variant<>](markdown/src/ch16.md)
  - [第十七章 std::any](markdown/src/ch17.md)
  - [第十八章 std::byte](markdown/src/ch18.md)
  - [第十九章 字符串视图](markdown/src/ch19.md)
  - [第二十章 文件系统库](markdown/src/ch20.md)
- 第四部分 已有标准库的扩展和修改
  - [第二十一章 类型特征扩展](markdown/src/ch21.md)
  - [第二十二章 并行 STL 算法](markdown/src/ch22.md)
  - [第二十三章 新的 STL 算法详解](markdown/src/ch23.md)
  - [第二十四章 子串和子序列搜索器](markdown/src/ch24.md)
  - [第二十五章 其他工具函数和算法](markdown/src/ch25.md)
  - [第二十六章 容器和字符串扩展](markdown/src/ch26.md)
  - [第二十七章 多线程和并发](markdown/src/ch27.md)
  - [第二十八章 标准库的其他微小特性和修改](markdown/src/ch28.md)
- 第五部分 专家的工具
  - [第二十九章 多态内存资源 (PMR)](markdown/src/ch29.md)
  - [第三十章 使用 new 和 delete 管理超对齐数据](markdown/src/ch30.md)
  - [第三十一章 std::to_chars() 和 std::from_chars()](markdown/src/ch31.md)
  - [第三十二章 std::launder()](markdown/src/ch32.md)
  - [第三十三章 编写泛型代码的改进](markdown/src/ch33.md)
- 第六部分 一些通用的提示
  - [第三十四章 总体性的 C++17 事项](markdown/src/ch34.md)
  - [第三十五章 废弃和移除的特性](markdown/src/ch35.md)
  
---

## 说明

markdown 版是用程序直接从 pdf 版转换而来，然后再进行校对和修正。

但转换过程中不仅损失了一些信息，还导致了很多错误。

校对时虽然修正了很多错误，但因为时间有限，一定还有很多错误没有发现。

**因此更加推荐 pdf 版**

不管是 pdf 版还是 markdown 版，如果发现错误欢迎指出，也可以自行修正之后提交PR。

## 编译

1. 安装`tectonic`
2. 确保有所需字体：宋体、楷体（可选）、Times New Roman、Consolas
3. 如果`latex/src/main.tex`中有`\includeonly{xxx}`这一行且未被注释掉（%后的内容是注释），请在编译之前删除这一行
4. 编译pdf文件
    ```
    cd latex/src
    tectonic main.tex
    ```
5. 生成的`main.pdf`就是最后的pdf文件

- note: 如果安装`tectonic`之后`tectonic`不在`PATH`中，请把第5步中的`tectonic`替换成完整的路径
- note: 如果希望进行替换字体或修改行距等操作，可以修改`latex/src/main.tex`
- note: 如果所有步骤都正确操作仍编译失败，欢迎提issue
