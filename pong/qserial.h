#ifndef QSERIAL_H
#define QSERIAL_H
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QQueue>
#include <QTextStream>

class QSerial : public QObject
{
public:
    explicit QSerial(QString, int = 9600, bool = true, bool = false);
    ~QSerial();

    bool open();
    void close();

    /*
     * envia por puerto serie el mensaje del array pasado por parametro
     * Se espera una respuesta por defaul 200ms
     */
    bool serialSend(quint8 *, int, int = 200);

    /*
     * serialAvailable
     * retorna 0 si no se han recibido datos
     * retorna -1 si no se han recibido datos pero aún no se ha cumplido el timeout
     * retorna la cantidad de bytes disponibles
     */
    int serialAvailable();

    /*
     * copia los datos recibidos en el array. El tamaño se puede obtener con serialAvailable()
     */
    void serialReceive(quint8 *);


private slots:
    void OnQSerialPort1Rx();
    void onQTimer10ms();


private:
    QSerialPort *QSerialPort1;
    QTimer *QTimer10ms;
    // fingerprint del header, como constante ("UNER00:")
    const quint8 header[7] = {0x55, 0x4E, 0x45, 0x52, 0x00, 0x00, 0x3A};
    // Queue con los datos en raw recibidos en el puerto serie
    QQueue<quint8> rx;
    quint8 rxStage;
    quint8 payloadSize;
    quint8 payloadCount;
    // Comando recibido
    QVector<quint8> rcv;
    // tiempo para completar la respuesta a un comando enviado a arduino
    int responseTimeout;
    // tiempo para terminar de recibir un comando que se ha empezado a recibir
    int cmdTimeout;
    // true si se ha recibido un comando
    bool onCMD;
    // debug
    bool debug;

    // opciones del puerto
    bool serialReadWrite;
    int serialBaudRate;
    QString serialPortName;
    ////////////////////////////////////////////////////////
    // Funciones privadas
    /*
     * calcula chechsum xor
     */
    quint8 checksum(quint8 *, int);

    /* Procesa los datos en el buffer de entrada y guarda los datos si están OK
     * No sigue procesando hasta que no se han leído los datos existentes con serialReceive
     */
    void checkHeader();
};
#endif // QSERIAL_H
