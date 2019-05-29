#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <windows.h>
#include <QSet>
#include <QChar>
#include <QFile>
#include <QTime>
#include <QDebug>
#include <QTimer>
#include <QVector>
#include <QProcess>
#include <QDateTime>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>

static int EndNumber=114514;
static QString FileName;
static QSet<QString>ExistSet;

namespace FileOperation {
    QVector<QString> ReadFile() {
        QVector<QString>re;
        QFile file(FileName);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            return re;
        }
        QTextStream in(&file);
        QString all=in.readAll();
        std::string now;
        for(int i=0;i<all.length();i++) {
            int tmp=0,flag=1;
            while(!all[i].isNumber()&&all[i]!=QChar('-')&&i<all.length()) {
                i++;
            }
            if(i>all.length()) {
                break;
            }
            if(all[i]==QChar('-')) {
                flag=-1;
                i++;
            }
            while(all[i].isNumber()&&i<all.length()) {
                tmp=tmp*10+(all[i].toLatin1()-'0');
                i++;
            }
            tmp*=flag;
            if(tmp==EndNumber) {
                re.push_back(QString::fromStdString(now));
                now.clear();
            }
            else {
                now+=char(tmp);
            }
        }
        file.close();
        return re;
    }
    bool SaveFile(Ui::MainWindow *ui) {
        QFile file(FileName);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            return false;
        }
        QTextStream out(&file);
        int tot=ui->SongsList->count();
        for(int i=0;i<tot;i++) {
            std::string now=ui->SongsList->item(i)->text().toStdString();
            for(unsigned long long j=0;j<now.length();j++) {
                out<<int(now[j])<<' ';
            }
            out<<EndNumber<<endl;
        }
        file.close();
        return true;
    }
    void TranslateToListWidget(QVector<QString>data,Ui::MainWindow *ui) {
        ExistSet.clear();
        ui->SongsList->clear();
        for(int i=0;i<data.size();i++) {
            QString now=data[i];
            int pos=now.indexOf(" - ");
            QString SongName,SingerName,RealName;
            if(pos==-1) {
                SongName=now;
            }
            else {
                for(int j=0;j<pos;j++) {
                    SongName.append(now[j]);
                }
                for(int j=pos+3;j<now.length();j++) {
                    SingerName.append(now[j]);
                }
            }
            for(int j=0;j<SongName.length();j++) {
                if(!SongName[j].isSpace()) {
                    if(SongName[j].isUpper()) {
                        RealName.append(SongName[j].toLower());
                    }
                    else {
                        RealName.append(SongName[j]);
                    }
                }
            }
            if(ExistSet.empty()||ExistSet.find(RealName)==ExistSet.constEnd()) {
                ExistSet.insert(RealName);
                if(SingerName.length()) {
                    ui->SongsList->addItem(SongName+" - "+SingerName);
                }
                else {
                    ui->SongsList->addItem(SongName);
                }
            }
        }
        return ;
    }
}

namespace Log {
    static QString LogName="DDSAR.log";
    static QFile LogFile;
    static QTextStream log(&LogFile);
    void init() {
        LogFile.setFileName(LogName);
        LogFile.open(QIODevice::ReadOnly|QIODevice::Text);
        QString old=log.readAll();
        LogFile.close();
        LogFile.open(QIODevice::WriteOnly|QIODevice::Text);
        log<<old;
    }
    void Write(QString TypeMessage,QString TextMessage) {
        QString TimeMessage=QDateTime::currentDateTime().toString("[yyyy/MM/dd/hh:mm:ss]");
        log<<'['<<TypeMessage<<']'<<TimeMessage<<TextMessage<<endl;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    QProcess process;
    process.start("tasklist.exe");
    process.waitForFinished();
    QString ProcessName=QString::fromLocal8Bit(process.readAllStandardOutput());
    if(ProcessName.count("DDSAR.exe")>=2) {
        this->StartOK=false;
        QMessageBox::information(this,"警告","已有相同程序正在运行");
        return ;
    }

    ui->setupUi(this);
    Log::init();

    //随机数初始化
    srand(uint(time(nullptr)));
    Log::Write("OPRT","Rand seed changes successfully.");

    //读取歌曲信息
    FileName="Data.ddt";
    QVector<QString>data=FileOperation::ReadFile();
    if(data.empty()) {
        QMessageBox::information(this,"警告","数据无法读取");
        Log::Write("WARN","Open \""+FileName+"\" failed!");
        return ;
    }
    FileOperation::TranslateToListWidget(data,ui);
    Log::Write("OPRT","Open \""+FileName+"\".");
    connect(timer,SIGNAL(timeout()),this,SLOT(GetRandom()));
}

MainWindow::~MainWindow() {
    if(!(this->StartOK)) {
        delete ui;
    }
    //保存歌曲信息
    else {
        if(!FileOperation::SaveFile(ui)) {
            Log::Write("WARN","Save data failed when closing DDSAR!");
            QMessageBox::information(this,"警告","数据无法写入");
        }
        Log::Write("OPRT","Close DDSAR.");
        delete ui;
    }
}

//添加歌曲
void MainWindow::on_AddSong_clicked() {
    QString SongName=ui->SongName->text(),SingerName=ui->SingerName->text(),RealName;
    for(int i=0;i<SongName.length();i++) {
        if(!SongName[i].isSpace()) {
            if(SongName[i].isUpper()) {
                RealName.append(SongName[i].toLower());
            }
            else {
                RealName.append(SongName[i]);
            }
        }
    }
    if(ExistSet.find(RealName)==ExistSet.constEnd()) {
        ExistSet.insert(RealName);
        if(SingerName.length()) {
            ui->SongsList->addItem(SongName+" - "+SingerName);
            Log::Write("OPRT","Add song \""+SongName+" - "+SingerName+"\".");
        }
        else {
            ui->SongsList->addItem(SongName);
            Log::Write("OPRT","Add song \""+SongName+"\".");
        }
    }
    else {
        if(SingerName.length()) {
            Log::Write("WARN","Add existed song \""+SongName+" - "+SingerName+"\"!");
        }
        else {
            Log::Write("WARN","Add existed song \""+SongName+"\"!");
        }
        QMessageBox::information(this,"警告","有重复歌曲");
    }
    return ;
}

//删除歌曲
void MainWindow::on_DeleteSong_clicked() {
    QString SongName=ui->SongsList->currentItem()->text(),RealName;
    int len=SongName.length();
    for(int i=0;i<len-2;i++) {
        if(SongName[i]==QChar(' ')&&SongName[i+1]==QChar('-')&&SongName[i+2]==QChar(' ')) {
            len=i;
            break;
        }
    }
    for(int i=0;i<len;i++) {
        if(!SongName[i].isSpace()) {
            if(SongName[i].isUpper()) {
                RealName.append(SongName[i].toLower());
            }
            else {
                RealName.append(SongName[i]);
            }
        }
    }
    if(ExistSet.find(RealName)!=ExistSet.constEnd()) {
        ExistSet.remove(RealName);
    }
    ui->SongsList->takeItem(ui->SongsList->currentRow());
    Log::Write("OPRT","Delete song \""+ui->SongsList->currentItem()->text()+"\".");
    return ;
}

//随机歌曲
void MainWindow::on_GetRandom_clicked() {
    if(ui->GetRandom->text()=="随机") {
        ui->SongName->clear();
        ui->SongName->setDisabled(true);
        ui->SingerName->clear();
        ui->SingerName->setDisabled(true);
        ui->AddSong->setDisabled(true);
        ui->DeleteSong->setDisabled(true);
        ui->MakeRandom->setDisabled(true);
        timer->start(75);
        ui->GetRandom->setText("暂停");
        Log::Write("OPRT","Rand begins.");
    }
    else {
        timer->stop();
        ui->SongName->setEnabled(true);
        ui->SingerName->setEnabled(true);
        ui->AddSong->setEnabled(true);
        ui->DeleteSong->setEnabled(true);
        ui->MakeRandom->setEnabled(true);
        ui->GetRandom->setText("随机");
        Log::Write("OPRT","Rand result: \""+ui->SongsList->currentItem()->text()+"\".");
    }
    return ;
}

//选中歌曲
void MainWindow::on_SongsList_itemSelectionChanged() {
    if(ui->SongsList->currentRow()!=-1) {
        ui->CopyBoard->setText(ui->SongsList->currentItem()->text());
    }
    else {
        ui->CopyBoard->setText("");
    }
    return ;
}

//打乱歌曲
void MainWindow::on_MakeRandom_clicked()
{
    int tot=ui->SongsList->count();
    QVector<QString>tmp;
    for(int i=0;i<tot;i++) {
        tmp.push_back(ui->SongsList->item(i)->text());
    }
    std::random_shuffle(tmp.begin(),tmp.end());
    ui->SongsList->clear();
    for(int i=0;i<tot;i++) {
        ui->SongsList->addItem(tmp[i]);
    }
    Log::Write("OPRT","Shuffle succeeds.");
    return ;
}

//打开文件
void MainWindow::on_ActionFileIn_triggered() {
    if(!FileOperation::SaveFile(ui)) {
        QMessageBox::information(this,"警告","数据无法写入");
        Log::Write("WARN","Save file \""+FileName+"\" failed!");
    }
    QString TmpFileName=FileName;
    FileName=QFileDialog::getOpenFileName(this,tr("Open File"),".","DDSAR Files(*.ddt)");
    QVector<QString>data=FileOperation::ReadFile();
    if(data.empty()) {
        QMessageBox::information(this,"警告","数据无法读取");
        Log::Write("WARN","File \""+FileName+"\" is empty!");
        FileName=TmpFileName;
        return ;
    }
    FileOperation::TranslateToListWidget(data,ui);
    Log::Write("OPRT","Open file \""+FileName+"\".");
    return ;
}

//保存文件
void MainWindow::on_ActionFileOut_triggered() {
    QString TmpFileName=FileName;
    FileName=QFileDialog::getOpenFileName(this,tr("Save File"),".","DDSAR Files(*.ddt)");
    if(!FileOperation::SaveFile(ui)) {
        QMessageBox::information(this,"警告","数据无法写入");
        Log::Write("WARN","Save file \""+FileName+"\" failed!");
        FileName=TmpFileName;
        return ;
    }
    Log::Write("OPRT","Save file \""+FileName+"\".");
    return ;
}

//新旧文件转换
//TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
void MainWindow::on_ChangeFile_triggered() {
    QMessageBox::information(this,"Warning","暂不可用（也就是coder还没打这个代码）");
    Log::Write("WTF?","Someone has found the egg.");
    return ;
/**
    QString TmpFileName=QFileDialog::getOpenFileName(this,tr("Save File"));
    QFile file(TmpFileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QMessageBox::information(this,"警告","数据无法转换");
        return ;
    }
    QTextStream in(&file);
    QString data=in.readAll();
    freopen(TmpFileName.toStdString().c_str(),"w",stdout);
    for(int i=0;i<data.length();i++) {
        std::cout<<char(data[i].unicode()^secret);
    }
    file.close();
    return ;
*/
}

//随机过程
void MainWindow::GetRandom() {
    qsrand(uint(QTime(0,0,0,0).msecsTo((QTime::currentTime()))));
    int rnd=qrand()%ui->SongsList->count();
    ui->SongsList->setCurrentRow(rnd);
}
