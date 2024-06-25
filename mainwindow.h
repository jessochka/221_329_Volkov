#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>
#include <QVector>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addTransaction(const QString &transaction, bool highlight = false);
    void loadTransactionsFromFile(const QString &filePath);

private slots:
    void on_pushButtonOpen_clicked();

private:
    Ui::MainWindow *ui;
    QString calculateHash(const QString &amount, const QString &wallet, const QString &date, const QString &previousHash);
};

#endif // MAINWINDOW_H
