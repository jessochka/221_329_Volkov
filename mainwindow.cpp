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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Загружаем транзакции из файла при старте приложения
    loadTransactionsFromFile("C:\\Users\\Sema\\Desktop\\221_329_Volkov\\transactions.csv");

    // Подключаем кнопку "Открыть" к слоту
    connect(ui->pushButtonOpen, &QPushButton::clicked, this, &MainWindow::on_pushButtonOpen_clicked);
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
    QFile file(filePath);
    if (!file.exists()) {
        QMessageBox::warning(this, "Ошибка", "Файл не существует: " + filePath);
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл с транзакциями: " + filePath);
        return;
    }

    QTextStream in(&file);
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

    file.close();
}

void MainWindow::on_pushButtonOpen_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "CSV Files (*.csv);;All Files (*)");
    if (!filePath.isEmpty()) {
        loadTransactionsFromFile(filePath);
    }
}
