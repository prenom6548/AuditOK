#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>


// serial:/dev/ttyS0?baud=115200+bits=8+parity=odd+flow=none
class SerialConfigHelper {
public:
    // 默认参数配置
    struct SerialParams {
        QString device{QString()};
        QString baudrate{QString()};
        QString bytesize{QString()};
        QString parity{QString()};
        QString flowcontrol{QString()};
    };

    // 补全并修改配置字符串
    static QString modifyBaudrate(const QString &config, const QString &newBaudrate) {
        // 1. 解析原始配置
        SerialParams params = parseConfig(config);
        
        // 2. 更新波特率
        params.baudrate = newBaudrate;
        
        // 3. 生成新的配置字符串
        return buildConfigString(params);
    }

private:
    // 解析配置字符串（自动补全缺失参数）
    static SerialParams parseConfig(const QString &config) {
        SerialParams params;
        
        // 提取设备路径
        QRegularExpression deviceRegex("^serial:([^?]+)");
        QRegularExpressionMatch deviceMatch = deviceRegex.match(config);
        if (deviceMatch.hasMatch()) {
            params.device = deviceMatch.captured(1);
        }
        
        // 提取查询参数部分
        int queryStart = config.indexOf('?');
        if (queryStart != -1) {
            QString queryStr = config.mid(queryStart + 1).replace('+', '&');
            QUrlQuery query(queryStr);
            
            // 解析波特率
            if (query.hasQueryItem("baud")) {
                params.baudrate = query.queryItemValue("baud");
            }
            
            // 解析数据位
            if (query.hasQueryItem("bits")) {
                params.bytesize = query.queryItemValue("bits");
            }
            
            // 解析校验位
            if (query.hasQueryItem("parity")) {
                params.parity = query.queryItemValue("parity");
            }
            
            // 解析流控
            if (query.hasQueryItem("flow")) {
                params.flowcontrol = query.queryItemValue("flow");
            }
        }
        
        return params;
    }

    // 构建配置字符串
    static QString buildConfigString(const SerialParams &params) {
        QUrlQuery query;
        if (!params.baudrate.isEmpty()) {
            query.addQueryItem("baud", params.baudrate);
        }
        if (!params.bytesize.isEmpty()) {
            query.addQueryItem("bits", params.bytesize);
        }
        if (!params.parity.isEmpty()) {
            query.addQueryItem("parity", params.parity);
        }
        if (!params.flowcontrol.isEmpty()) {
            query.addQueryItem("flow", params.flowcontrol);
        }
        return QString("serial:%1?%2").arg(params.device, 
                                         query.toString(QUrl::FullyEncoded).replace('&', '+'));
    }
};

// 使用示例
// int main(int argc, char *argv[]) {
//     QCoreApplication a(argc, argv);

//     // 测试各种输入情况
//     QString testCases[] = {
//         "serial:/dev/ttyS0",  // 最短形式
//         "serial:/dev/ttyUSB0?baudrate=9600",  // 部分参数
//         "serial:/dev/ttyACM0?baudrate=115200+bytesize=8",  // 部分参数
//         "serial:/dev/ttyS1?baudrate=57600+bytesize=8+parity=odd+flowcontrol=hardware"  // 完整参数
//     };

//     for (const QString &testCase : testCases) {
//         qDebug() << "Original:" << testCase;
//         QString result = SerialConfigHelper::modifyBaudrate(testCase, 19200);
//         qDebug() << "Modified:" << result;
//         qDebug() << "----------------------------------";
//     }

//     return a.exec();
// }