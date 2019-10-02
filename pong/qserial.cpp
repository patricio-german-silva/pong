#include "qserial.h"



/*!
 * \brief QSerial::QSerial
 * Implementa el manejo del protocolo de comunicación por puerto serie con arduino
 * Por protocolo, los mensajes desde la PC al arduino siempre son respondidos
 * Si no se establece un periodo de respuesta en la llamada a serialSend() se utiliza por defecto 50ms
 * Los mensajes del arduino al PC no requieren respuesta
 * Los mensajes no tienen numero de secuencia por lo que se debe realizar los controles necesarios para
 * interpretar las respuestas correctamente.
 * \param portName es el nombre del puerto serie
 * \param br indica el baud rate
 * \param rw indica apertura en solo lectura o lectura/escritura
 * \param dbg indica si se activa el debug
 */
QSerial::QSerial(QString portName, int br, bool rw, bool dbg)
{
    QSerialPort1 = new QSerialPort(this);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &QSerial::OnQSerialPort1Rx);
    serialPortName = portName;
    serialBaudRate = br;
    serialReadWrite = rw;
    debug = dbg;
    QTimer10ms = new QTimer();
    connect(QTimer10ms, &QTimer::timeout, this, &QSerial::onQTimer10ms);
    rxStage = 0;
    payloadSize = 0;
    payloadCount = 0;
    cmdTimeout = 0;
    responseTimeout = 0;
    rx.clear();
    rcv.clear();
    QTimer10ms->setInterval(10);
    QTimer10ms->start();
}

QSerial::~QSerial()
{
    QSerialPort1->close();
    delete QSerialPort1;
}

bool QSerial::open(){
    QSerialPort1->setPortName(serialPortName);
    QSerialPort1->setBaudRate(serialBaudRate);
    QSerialPort1->open(serialReadWrite?QSerialPort::ReadWrite:QSerialPort::ReadOnly);
}

void QSerial::close(){
    QSerialPort1->close();
}

void QSerial::onQTimer10ms(){
    if(cmdTimeout)
        cmdTimeout--;
    else if(rxStage){
        rxStage=0;
        rcv.clear();
        rx.clear();
    }
    if(responseTimeout)
        responseTimeout--;

}

// Interrupción - Recibir datos
void QSerial::OnQSerialPort1Rx(){
    int count;
    quint8 *buf;
    count = (quint8)QSerialPort1->bytesAvailable();
    if(count<=0)
        return;
    buf = new quint8[count];
    QSerialPort1->read((char *)buf, count);
    for (int i = 0; i<count; i++)
        rx.enqueue(buf[i]);
    if(debug){
        QTextStream out(stdout);
        out << "[ 0x";
        for(int i = 0; i<count; i++)
            out << QString("%1").arg(buf[i], 2, 16, QChar('0')).toUpper();
        out << "]" << endl;
    }
    delete[] buf;
    checkHeader();
}

// calculo checksum XOR
quint8 QSerial::checksum(quint8 *buf, int length){
    quint8 chk;
    chk = buf[0];
    for (int i = 1 ; i<length; i++){
        chk ^= buf[i];
    }
    return chk;
}

/*
 * Envia message por puerto serie agregando header y checksum
 * establece la espera de una respuesta a timeout milisegundos
 */
bool QSerial::serialSend(quint8 *message, int size, int timeout){
    if (QSerialPort1->isOpen()){
        responseTimeout=timeout/10;
        quint8 *buf = new quint8[size+8];
        for (int i = 0; i < 7; i++){
            buf[i] = header[i];
        }
        buf[4] = (quint8)(size+1);
        buf[5] = (quint8)((size+1)/512);
        for(int i = 0;i < size; i++){
            buf[i+7] = message[i];
        }
        buf[size+7] = QSerial::checksum(buf, size+7);
        QSerialPort1->write((char *)buf, size+8);
        if(debug){
            QTextStream out(stdout);
            out << "<-- 0x";
            for(int i = 0; i<size+8; i++)
                out << QString("%1").arg(buf[i], 2, 16, QChar('0')).toUpper();
            out << endl;
        }
        delete [] buf;
        return true;
    }
    return false;
}


// Chequeo si hay un comando valido en el buffer de entrada rx
void QSerial::checkHeader(){
    while (!rx.empty() && !onCMD){
        switch(rxStage){
        case 0:
            cmdTimeout=5;
            rcv.clear();
        case 1:
        case 2:
        case 3:
        case 6:
            if(rx.head() == header[rxStage]){
                rcv.append(rx.dequeue());
                rxStage++;
            }else{
                if(!rxStage) rx.dequeue();
                rxStage = 0;
            }
            break;
        case 4:
            rcv.append(rx.head());
            payloadSize = rx.dequeue();
            rxStage++;
            break;
        case 5:
            rcv.append(rx.head());
            payloadSize += rx.dequeue()*256;
            payloadCount = payloadSize;
            rxStage++;
            break;
        case 7:
            if(payloadCount-- > 1){
                rcv.append(rx.dequeue());
            }else{
                if(rx.dequeue() == QSerial::checksum(rcv.data(), payloadSize+6)){
                    rxStage=0;
                    onCMD = true;
                    if(debug){
                        QTextStream out(stdout);
                        out << "--> 0x";
                        for(int i = 0; i<rcv.length(); i++)
                            out << QString("%1").arg(rcv.data()[i], 2, 16, QChar('0')).toUpper();
                        out << QString("%1").arg(QSerial::checksum(rcv.data(), payloadSize+6), 2, 16, QChar('0')).toUpper();
                        out << endl;
                    }
                }else{
                    rxStage=0;
                    onCMD = false;
                    rcv.clear();
                }
            }
            break;
        default:
            rxStage=0;
            onCMD=false;
        }
    }
}

/*
 * serialAvailable
 * retorna 0 si no se han recibido datos
 * retorna -1 si no se han recibido datos pero aún no se ha cumplido el timeout
 * retorna la cantidad de bytes disponibles
 */
int QSerial::serialAvailable(){
    checkHeader();
    if(onCMD)
        return rcv.length()-7;
    else if(responseTimeout)
        return -1;
    return 0;
}


/*
 * copia los datos recibidos en el array. El tamaño se puede obtener con serialAvailable()
 */
void QSerial::serialReceive(quint8 *b){
    checkHeader();
    if(onCMD){
        for (int i=0; i<rcv.length()-7; i++)
            b[i]=rcv.data()[i+7];
        onCMD=false;
        rcv.clear();
    }
}
