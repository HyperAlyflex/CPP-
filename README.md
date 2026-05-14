### 这里Mac端里面有已经打包好的五子棋
### Sth是开发的源码，CPP开发，Qt界面

### 功能说明

本项目目前实现的功能包括：

- 双人轮流落子
- 五子连珠胜负判定
- 悔棋功能
- 新对局重置
- 双方倒计时
- 超时判负
- 鼠标悬停预落子提示
- 简化禁手：黑棋长连判负

### 开发环境
- 开发语言：C++
- 图形界面库：Qt 6
- 构建方式：qmake
- 开发工具：Qt Creator


### 文件结构

- `main.cpp`：程序入口
- `mainwindow.h / mainwindow.cpp / mainwindow.ui`：主窗口界面与交互控制
- `boardwidget.h / boardwidget.cpp`：棋盘绘制与鼠标交互
- `gomokugame.h / gomokugame.cpp`：游戏核心逻辑
