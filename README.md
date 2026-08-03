# DataMonitor

工业数据采集与监控系统（SCADA Lite）— Qt 6 + C++17 上位机面试作品。

## 技术栈

| 层      | 技术 |
|---------|------|
| 框架    | Qt 6.11 + C++17 + CMake |
| 通信    | QSerialPort (Modbus RTU) + QTcpSocket (Modbus TCP) |
| 图表    | Qt Charts (QLineSeries + QValueAxis) |
| 数据    | SQLite (QSqlDatabase) |
| 线程    | QThread + moveToThread |
| 构建    | CMake + MinGW 13.1.0 |

## 功能特性

### ✅ 已实现

- **串口通信** — Modbus RTU 协议 (QSerialPort)，支持 CRC-16/Modbus 校验
- **TCP 通信** — Modbus TCP 协议 (QTcpSocket)，与串口共用协议解析层
- **协议解析** — 地址/功能码可配置，帧数据自动校验
- **多线程架构** — 通信线程 + 数据库写入线程分离，不阻塞 UI
- **多通道支持** — 4 通道同时监测 (0x01-0x04)，实时曲线多色显示
- **实时曲线** — Qt Charts 折线图，X 轴 200 点滚动窗口，Y 轴自动缩放
- **实时列表** — 右 Dock 滚动显示最新 100 条数据，超阈值报警标红
- **数据入库** — SQLite 自动保存，历史数据 Tab 页支持查询
- **断线重连** — 串口断开后每 3 秒自动重试
- **配置保存** — QSettings 记住上次串口参数
- **配置对话框** — 串口/TCP 双模式 Tab 页，连接状态反馈
- **文件日志** — 实时写入 data_monitor.log（线程安全）
- **CSV 导出** — 曲线数据导出为标准 CSV
- **QSS 样式** — 蓝色调主题
- **断线检测** — 串口 ResourceError 自动识别并触发重连

### 📋 规划中

- **国际化** — 英文界面支持

## 项目结构

```
DataMonitor/
├── CMakeLists.txt
├── app/main.cpp
├── core/          # 核心逻辑：Crc16, ProtocolParser, CommunicationManager, TcpCommunicationManager, FileLogger
├── thread/        # 工作线程：CommWorker, DatabaseWriter
├── db/            # 数据库：DatabaseManager
├── ui/            # 界面：MainWindow, SettingsDialog
└── resources/     # 资源：QSS 样式（.qrc 内嵌）
```

## 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=D:/Qt/6.11.0/mingw_64
cmake --build .
```

## 协议说明

串口（Modbus RTU）与 TCP 共用同一套解析层，帧为固定 6 字节：

```
[地址][功能码][数值低字节][数值高字节][CRC低字节][CRC高字节]
```

- 数值按小端解析；CRC-16/Modbus 按低字节在前存放
- `ProtocolParser` 构造参数可指定设备地址列表（默认 0x01–0x04）和功能码
- `CommunicationManager`（串口）与 `TcpCommunicationManager`（TCP）均复用 `ProtocolParser` 和 `Crc16`
