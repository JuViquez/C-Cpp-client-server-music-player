#include "dialog.h"
#include "ui_dialog.h"
#include <QTime>
#include "playlistcontroller.h"
#include <vector>
#include <unistd.h>
#include <QPixmap>
#include <ctime>
#include <iostream>
#include <cstdlib>

Dialog::Dialog(QWidget *parent, PlaylistController *SuperObject) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    PLCObject = SuperObject;
    player = new QMediaPlayer(this);
    connect(player, &QMediaPlayer::positionChanged, this, &Dialog::on_positionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &Dialog::on_durationChanged);
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
    SetDefaultUI();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_3_clicked(){}
void Dialog::on_addPlaylist_clicked(){}

void Dialog::on_sliderProgress_sliderMoved(int position)
{
    player->setPosition(position);
}

void Dialog::on_sliderVolume_sliderMoved(int position)
{
    player->setVolume(position);
}

void Dialog::on_pushButton_clicked()
{
    if(ui->playlist->currentRow() > -1){
        string nextSong = ui->playlist->currentItem()->text().toLatin1().data();
        player->stop();
        if(!nextSong.compare(PLCObject->currentSong)){
            player->play();
        }else{
            PLCObject->RemoveSongFile();
            if(ui->playlist->currentItem()){
                PLCObject->currentIndexSong = ui->playlist->currentRow();
                PLCObject->currentSong = nextSong;
                if(PLCObject->GetSong()>0)
                    LoadSong();
            }
        }
    }
}

void Dialog::LoadSong(){
    QPixmap pic(PLCObject->currentImage.c_str());
    ui->imageLabel->setPixmap(pic.scaled(ui->imageLabel->width(),ui->imageLabel->height(),Qt::KeepAspectRatio));
    QString applicationPath = QCoreApplication::applicationDirPath();
    QString fileName = QString::fromStdString("/"+PLCObject->currentSong);
    player->setMedia(QUrl::fromLocalFile(applicationPath.toUtf8().constData()+fileName));
    player->play();
    qDebug() << player->errorString();
}

void Dialog::on_pushButton_2_clicked()
{
    player->stop();
}

void Dialog::on_positionChanged(qint64 position)
{
    ui->sliderProgress->setValue(position);
    seconds = (position/1000) % 60;
    minutes = (position/60000) % 60;
    timeDuration = new QTime(0, minutes,seconds);
    ui->positionLabel->setText(timeDuration->toString());
}

void Dialog::on_durationChanged(qint64 position)
{
    ui->sliderProgress->setMaximum(position);
    int seconds2 = (position/1000) % 60;
    int minutes2 = (position/60000) % 60;
    QTime* TotalDuration = new QTime(0, minutes2,seconds2);
    ui->totalLabel->setText(TotalDuration->toString());
}

void Dialog::on_BtnAdd_clicked()
{
    if(ui->panel->count()){
        ui->playlist->addItem(ui->panel->currentItem()->text());
        PLCObject->queueplayList.push_back(ui->panel->currentItem()->text().toUtf8().constData());
        //PLCObject.GetSong(ui->panel->currentItem()->text().toLatin1().data());
    }
}

void Dialog::mediaStatusChanged(QMediaPlayer::MediaStatus state)
{
    if(state==QMediaPlayer::EndOfMedia)
    {
        if(ui->randomCheck->isChecked() && PLCObject->queueplayList.size()){
            SetRandomIndex();
        }
        if(PLCObject->PlayNextSong() > 0){
            LoadSong();
            ui->playlist->setCurrentRow(PLCObject->currentIndexSong);
        }
    }
}

void Dialog::on_Remove_clicked()
{
    if(ui->playlist->count()){
        PLCObject->RemoveSongFromPlaylist(ui->playlist->currentRow());
        ui->playlist->clear();
        for(std::string &s : PLCObject->queueplayList) {
            ui->playlist->addItem(s.c_str());
        }
    }
}

void Dialog::on_BtnRefresh_clicked()
{
    if(PLCObject->GetSongsList()>0){
        ui->panel->clear();
        for(std::string &s : PLCObject->songsList) {
            ui->panel->addItem(s.c_str());
        }
    }else{
        //ui->StatusLabel->setText("Error: Listing files failed");
    }
}

void Dialog::on_BtnShuffle_clicked()
{
    if(PLCObject->queueplayList.size()){
        PLCObject->ShufflePLaylist();
        ui->playlist->clear();
        for(std::string &s : PLCObject->queueplayList) {
            ui->playlist->addItem(s.c_str());
        }
        ui->playlist->setCurrentRow(PLCObject->currentIndexSong);
    }
}

void Dialog::on_BtnPause_clicked()
{
    if(player->state() == QMediaPlayer::PlayingState){
        player->pause();
        ui->BtnPause->setText("Resume");
    }else if(player->mediaStatus() > 2 ){
        player->play();
        ui->BtnPause->setText("Pause");
    }
}


void Dialog::on_BtnNext_clicked()
{
    if(ui->randomCheck->isChecked() && PLCObject->queueplayList.size()){
        SetRandomIndex();
    }
    if(PLCObject->PlayNextSong() > 0){
        LoadSong();
        ui->playlist->setCurrentRow(PLCObject->currentIndexSong);
    }
}

void Dialog::on_BtnPrevious_clicked()
{
    if(PLCObject->PlayPreviousSong() > 0){
        LoadSong();
        ui->playlist->setCurrentRow(PLCObject->currentIndexSong);
    }
}

void Dialog::SetRandomIndex(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, PLCObject->queueplayList.size()-1);
    PLCObject->currentIndexSong = dis(gen);
}

void Dialog::SetDefaultUI(){
    //Background form
    QPixmap bkgnd("background.png");
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(palette);
    //Labels
    QPalette sample_palette;
    sample_palette.setColor(QPalette::WindowText, Qt::white);
    ui->positionLabel->setPalette(sample_palette);
    ui->totalLabel->setPalette(sample_palette);
    ui->backslashlabel->setPalette(sample_palette);

    //Stop
    QPixmap StopPixmap("../Buttons/Stop.png");
    QIcon StopIcon(StopPixmap);
    ui->pushButton_2->setIcon(StopIcon);
    //Start
    QPixmap StartPixmap("../Buttons/Play.png");
    QIcon StartIcon(StartPixmap);
    ui->pushButton->setIcon(StartIcon);
    //Previous
    QPixmap PreviousPixmap("../Buttons/Previous.png");
    QIcon PreviousIcon(PreviousPixmap);
    ui->BtnPrevious->setIcon(PreviousIcon);
    //Play
    QPixmap PausePixmap("../Buttons/Pause.png");
    QIcon PauseIcon(PausePixmap);
    ui->BtnPause->setIcon(PauseIcon);
    //Next
    QPixmap NextPixmap("../Buttons/Next.png");
    QIcon NextIcon(NextPixmap);
    ui->BtnNext->setIcon(NextIcon);
}
