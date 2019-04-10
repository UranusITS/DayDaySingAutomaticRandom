#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool StartOK=true;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_AddSong_clicked();

    void on_DeleteSong_clicked();

    void on_GetRandom_clicked();

    void on_SongsList_itemSelectionChanged();

    void on_MakeRandom_clicked();

    void on_ActionFileIn_triggered();

    void on_ActionFileOut_triggered();

    void on_ChangeFile_triggered();

    void GetRandom();

private:
    Ui::MainWindow *ui;
    QTimer *timer=new QTimer;
};

#endif // MAINWINDOW_H
