#include "dialog.h"
#include "ui_dialog.h"
#include <QTime>
#include "playlistcontroller.h"
#include <vector>
#include <unistd.h>
#include <QPixmap>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    PLCObject.SetSockets();
    connect(player, &QMediaPlayer::positionChanged, this, &Dialog::on_positionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &Dialog::on_durationChanged);
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

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
        if(!nextSong.compare(PLCObject.currentSong)){
            player->play();
        }else{
            PLCObject.RemoveSongFile();
            if(ui->playlist->currentItem()){
                PLCObject.currentIndexSong = ui->playlist->currentRow();
                PLCObject.currentSong = nextSong;
                if(PLCObject.GetSong()>0)
                    LoadSong();
            }
        }
    }
}

void Dialog::LoadSong(){
    QPixmap pic(PLCObject.currentImage.c_str());
    ui->imageLabel->setPixmap(pic.scaled(ui->imageLabel->width(),ui->imageLabel->height(),Qt::KeepAspectRatio));
    QString applicationPath = QCoreApplication::applicationDirPath();
    QString fileName = QString::fromStdString("/"+PLCObject.currentSong);
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
        PLCObject.queueplayList.push_back(ui->panel->currentItem()->text().toUtf8().constData());
        //PLCObject.GetSong(ui->panel->currentItem()->text().toLatin1().data());
    }
}

void Dialog::mediaStatusChanged(QMediaPlayer::MediaStatus state)
{
    if(state==QMediaPlayer::EndOfMedia)
    {
        if(PLCObject.PlayNextSong() > 0){
            LoadSong();
            ui->playlist->setCurrentRow(PLCObject.currentIndexSong);
        }
    }
}

void Dialog::on_Remove_clicked()
{
    if(ui->playlist->count()){
        PLCObject.RemoveSongFromPlaylist(ui->playlist->currentRow());
        ui->playlist->clear();
        for(std::string &s : PLCObject.queueplayList) {
            ui->playlist->addItem(s.c_str());
        }
    }
}

void Dialog::on_BtnRefresh_clicked()
{
    if(PLCObject.GetSongsList()>0){
        ui->panel->clear();
        for(std::string &s : PLCObject.songsList) {
            ui->panel->addItem(s.c_str());
        }
    }else{
        //ui->StatusLabel->setText("Error: Listing files failed");
    }
}

void Dialog::on_BtnShuffle_clicked()
{
    if(PLCObject.queueplayList.size()){
        PLCObject.ShufflePLaylist();
        ui->playlist->clear();
        for(std::string &s : PLCObject.queueplayList) {
            ui->playlist->addItem(s.c_str());
        }
        ui->playlist->setCurrentRow(PLCObject.currentIndexSong);
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
    if(PLCObject.PlayNextSong() > 0){
        LoadSong();
        ui->playlist->setCurrentRow(PLCObject.currentIndexSong);
    }
}

void Dialog::on_BtnPrevious_clicked()
{
    if(PLCObject.PlayPreviousSong() > 0){
        LoadSong();
        ui->playlist->setCurrentRow(PLCObject.currentIndexSong);
    }
}
