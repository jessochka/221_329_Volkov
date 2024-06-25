#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include <QFileDialog>
#include <openssl/aes.h>
#include <openssl/rand.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Загружаем транзакции из файла при старте приложения
    loadTransactionsFromFile("C:\\Users\\Sema\\Desktop\\221_329_Volkov\\transactions.csv");

    // Подключаем кнопки к слотам
    connect(ui->pushButtonOpen, &QPushButton::clicked, this, &MainWindow::on_pushButtonOpen_clicked);
    connect(ui->pushButtonUpdateKey, &QPushButton::clicked, this, &MainWindow::on_pushButtonUpdateKey_clicked);

    decryptionKey = ""; // Изначально ключ пустой
    iv = QByteArray::fromHex("5062c5972a7b9d71aaef87ea31451c22"); // Инициализация IV
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addTransaction(const QString &transaction, bool highlight)
{
    QTextCursor cursor = ui->plainTextEditTransactions->textCursor();
    cursor.movePosition(QTextCursor::End);

    if (highlight) {
        QTextCharFormat fmt;
        fmt.setBackground(Qt::red);
        cursor.insertText(transaction + "\n", fmt);
    } else {
        cursor.insertText(transaction + "\n");
    }

    ui->plainTextEditTransactions->setTextCursor(cursor);
}

QString MainWindow::calculateHash(const QString &amount, const QString &wallet, const QString &date, const QString &previousHash)
{
    QString data = amount + wallet + date + previousHash;
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

void MainWindow::loadTransactionsFromFile(const QString &filePath)
{
    QByteArray decryptedData = decryptFile(filePath, decryptionKey);
    if (decryptedData.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось расшифровать файл с транзакциями: " + filePath);
        return;
    }

    QString decryptedString = QString::fromUtf8(decryptedData);
    QTextStream in(&decryptedString);
    QString previousHash;
    int lineNumber = 0;
    bool highlight = false;

    ui->plainTextEditTransactions->clear(); // Очищаем перед загрузкой нового файла

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // Удаляем лишние пробелы
        lineNumber++;
        QStringList fields = line.split(",");
        if (fields.size() != 4) {
            QMessageBox::warning(this, "Ошибка", "Неверный формат данных в файле на строке " + QString::number(lineNumber) + ": " + line);
            return;
        }

        QString amount = fields[0].trimmed();
        QString wallet = fields[1].trimmed();
        QString date = fields[2].trimmed();
        QString hash = fields[3].trimmed();

        QString data = amount + wallet + date + previousHash;
        QString calculatedHash = calculateHash(amount, wallet, date, previousHash);

        qDebug() << "Line:" << lineNumber;
        qDebug() << "Data for hash:" << data;
        qDebug() << "Expected hash:" << hash;
        qDebug() << "Calculated hash:" << calculatedHash;

        if (calculatedHash != hash) {
            highlight = true;
        } else {
            highlight = false;
        }

        addTransaction(line, highlight);
        previousHash = hash;
    }
}

QByteArray MainWindow::decryptFile(const QString &filePath, const QString &key)
{
    if (key.isEmpty() || key.length() != 64) {
        QMessageBox::warning(this, "Ошибка", "Неверный ключ для расшифровки. Длина ключа должна быть 64 символа.");
        return QByteArray();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QByteArray encryptedData = file.readAll();
    file.close();

    AES_KEY decryptKey;
    QByteArray keyBytes = QByteArray::fromHex(key.toUtf8());
    if (AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(keyBytes.constData()), 256, &decryptKey) < 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось установить ключ для расшифровки.");
        return QByteArray();
    }

    QByteArray ivCopy = iv; // Сброс IV перед каждым новым расшифрованием

    QByteArray decryptedData(encryptedData.size(), 0);
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(encryptedData.constData()),
                    reinterpret_cast<unsigned char*>(decryptedData.data()),
                    encryptedData.size(),
                    &decryptKey,
                    reinterpret_cast<unsigned char*>(ivCopy.data()),
                    AES_DECRYPT);

    // Убираем padding
    int paddingLength = decryptedData.at(decryptedData.size() - 1);
    decryptedData.chop(paddingLength);

    return decryptedData;
}

void MainWindow::on_pushButtonOpen_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Encrypted Files (*.enc);;All Files (*)");
    if (!filePath.isEmpty()) {
        loadTransactionsFromFile(filePath);
    }
}

void MainWindow::on_pushButtonUpdateKey_clicked()
{
    decryptionKey = ui->lineEditKey->text();
    QMessageBox::information(this, "Ключ обновлен", "Ключ для расшифровки обновлен.");
}
