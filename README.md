# DataMonitor

工业数据采集与监控系统（SCADA Lite）— Qt 6 + C++17 上位机实习/面试作品。

## 技术栈

| 分类 | 技术 |
|------|------|
| 语言/框架 | C++17、Qt 6.11、CMake |
| 通信 | QSerialPort、QTcpSocket |
| 协议 | 自定义 Modbus 风格帧协议 + CRC-16/Modbus 校验 |
| 图表 | Qt Charts（QLineSeries + QValueAxis） |
| 数据存储 | SQLite（QSqlDatabase） |
| 多线程 | QThread + moveToThread |
| 配置 | QSettings |
| 测试 | Qt Test + CTest |

## 功能特性

### ✅ 已实现

- **双模式通信** — 串口与 TCP 切换，共用协议解析层
- **协议解析** — 地址/功能码可配置，CRC-16/Modbus 校验，异常帧自动跳过与重同步
- **多线程架构** — 通信收发与数据库写入在独立工作线程，通信与入库不阻塞 UI
- **多通道监控** — 4 通道（0x01–0x04）曲线同屏显示
- **实时曲线** — X 轴 200 点滚动窗口，Y 轴自动缩放
- **实时列表** — 最新 100 条数据滚动显示，超阈值报警标红
- **报警机制** — 数据超阈值实时标红 + 状态栏警报提示
- **数据入库** — SQLite 自动保存，历史数据 Tab 页查询展示
- **断线重连** — 串口/TCP 断开后每 3 秒自动重试
- **配置持久化** — QSettings 保存串口/TCP 参数，重启自动加载
- **配置对话框** — 串口/TCP 双模式 Tab 页，连接状态反馈
- **文件日志** — 线程安全写入 data_monitor.log
- **CSV 导出** — 曲线数据导出为标准 CSV
- **QSS 主题** — 蓝色调界面样式（.qrc 内嵌）

### 📋 规划中

- 国际化（英文界面）
- 报警阈值 UI 配置
- 标准 Modbus RTU/TCP 完整帧格式支持

## 项目结构

```
DataMonitor/
├── CMakeLists.txt
├── app/main.cpp              # 程序入口
├── core/                     # 核心逻辑：协议解析、校验、通信、日志
│   ├── Crc16.h/.cpp
│   ├── ProtocolParser.h/.cpp
│   ├── CommunicationManager.h/.cpp
│   ├── TcpCommunicationManager.h/.cpp
│   ├── ParsedFrame.h
│   └── FileLogger.h/.cpp
├── thread/                   # 工作线程
│   ├── CommWorker.h/.cpp
│   └── DatabaseWriter.h/.cpp
├── db/                       # 数据库访问
│   └── DatabaseManager.h/.cpp
├── model/                    # 数据模型
│   └── ChannelConfig.h
├── ui/                       # 界面
│   ├── MainWindow.h/.cpp/.ui
│   └── SettingsDialog.h/.cpp/.ui
├── resources/                # 资源
│   ├── resources.qrc
│   └── styles/style.qss
└── tests/                    # 单元测试
    ├── CMakeLists.txt
    ├── tst_crc16.cpp
    └── tst_protocolparser.cpp
```

## 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=D:/Qt/6.11.0/mingw_64
cmake --build .
```

## 测试

```bash
cd build
ctest
```

当前测试覆盖 `Crc16` 与 `ProtocolParser` 两个纯逻辑模块，包含标准校验向量、帧校验、分片、非法地址、CRC 错误、多帧粘连等场景。

## 配置

程序使用 `QSettings` 持久化运行参数：

- 串口：`port/name`、`port/baud`
- TCP：`tcp/host`、`tcp/port`
- 报警：`alarm/threshold`（默认 800）

## 协议说明

本项目使用**自定义 Modbus 风格帧协议**：帧结构参考 Modbus RTU，复用标准 CRC-16/Modbus 校验，但数据区格式为自定义简化上报，与标准 Modbus 存在差异。

帧格式（固定 6 字节）：

```
[地址][功能码][数值低字节][数值高字节][CRC低字节][CRC高字节]
```

- 数值按小端序解析
- CRC-16/Modbus 按低字节在前存放
- `ProtocolParser` 构造参数可指定设备地址列表（默认 0x01–0x04）与功能码
- `CommunicationManager`（串口）与 `TcpCommunicationManager`（TCP）均复用 `ProtocolParser` 和 `Crc16`