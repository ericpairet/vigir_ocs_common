#include "widget.h"
#include "ui_widget.h"
#include <time.h>
#include <sstream>
#include "QPlainTextEdit"
#include "QLineEdit"
#include "QMessageBox"
#include "boost/lexical_cast.hpp"
#include <iomanip>
#include <ctime>
#include "QSpinBox"

#define ROBOT_IP "192.168.130.103"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ros::start();

    ui->setupUi(this);
    ros::NodeHandle nh;
    ocs_logging_pub_ = nh.advertise<flor_ocs_msgs::OCSLogging>("/vigir_logging",        1, false);
    timer.start(33, this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::timerEvent(QTimerEvent *event)
{
    // check if ros is still running; if not, just kill the application
    if(!ros::ok())
        qApp->quit();

    //Spin at beginning of Qt timer callback, so current ROS time is retrieved
    ros::spinOnce();
}


void Widget::on_startButton_clicked()
{
    std::string dir = "/home/vigir/Experiments/";
    QRegExp rx("(\s\\\\)");
    std::string expName = (ui->experimentName->text().replace(rx,tr("_"))).toStdString();
    std::cout << "Exp name is " << expName << std::endl;
    boost::filesystem::path folder (std::string("/home/vigir/Experiments/"+expName));
    if(boost::filesystem::exists(folder))
    {
        std::cout << "Folder already exists " << folder.c_str() << std::endl;
        QMessageBox msg;
        msg.setWindowTitle(QString::fromStdString("Logging Error"));
        msg.setInformativeText(QString::fromStdString("Cannot have two experiments with the same name.\n\nPlease rename the experiment to continue."));
        msg.exec();
    }
    else
    {
        if(boost::filesystem::create_directory(folder))
            std::cout<< "Created new folder at " << folder.c_str() << std::endl;
        sendMsg(true);
    }
}

void Widget::on_stopButton_clicked()
{
    sendMsg(false);
}


void Widget::on_robotLogsButton_clicked()
{
    QRegExp rx("(\s\\\\)");
    std::string expName = (ui->experimentName->text().replace(rx,tr("_"))).toStdString();
    std::cout << "Exp name is " << expName << std::endl;
    boost::filesystem::path folder (std::string("/home/vigir/Experiments/"+expName));
    if(boost::filesystem::exists(folder))
        getRobotLogs(folder);
    else if(expName != "")
    {
        if(boost::filesystem::create_directory(folder))
            std::cout<< "Created new folder at " << folder.c_str() << std::endl;
        getRobotLogs(folder);
    }
    else
    {
        struct tm * timeinfo;
        time_t exptime;
        time(&exptime);
        timeinfo = localtime( &exptime);
        //std::string timeName = "/home/vigir/Experiments/"+boost::lexical_cast<std::string>(timeinfo->tm_mon)+"-"+boost::lexical_cast<std::string>(timeinfo->tm_mday)+"-"+boost::lexical_cast<std::string>(timeinfo->tm_year+1900);
        boost::filesystem::path folder ("/home/vigir/Experiments/"+boost::lexical_cast<std::string>(timeinfo->tm_mon)+"-"+boost::lexical_cast<std::string>(timeinfo->tm_mday)+"-"+boost::lexical_cast<std::string>(timeinfo->tm_year+1900));
        if(boost::filesystem::exists(folder))
            getRobotLogs(folder);
        else
        {
            if(boost::filesystem::create_directory(folder))
                std::cout<< "Created new folder at " << folder.c_str() << std::endl;
            getRobotLogs(folder);
        }
    }
}

void Widget::sendMsg(bool run)
{
    flor_ocs_msgs::OCSLogging msg;
    msg.run = run;
    msg.experiment_name = ui->experimentName->text().toStdString();
    msg.description = ui->descriptionBox->document()->toPlainText().toStdString();
    msg.ustream = ui->ustreamBox->isChecked();
    ocs_logging_pub_.publish(msg);
}

void Widget::on_experimentName_textChanged(const QString &arg1)
{
    if(ui->experimentName->text().toStdString() == "")
        ui->startButton->setEnabled(false);
    else
        ui->startButton->setEnabled(true);
}

void Widget::getRobotLogs(boost::filesystem::path folder)
{
    std::cout << "Getting robot logs....\nFirst making sure on the right network." << std::endl;
    std::string systemCall = "python /home/vigir/vigir_repo/catkin_ws/src/vigir_ocs_common/vigir_ocs_logging/src/atlas_log_downloader.py 192.168.130.103 "+folder.string()+" "+boost::lexical_cast<std::string>(ui->spinBox->value());
    std::cout << "calling the following... " << systemCall << std::endl;
    system(systemCall.c_str());
    std::cout << "Done Grabbing logs!" << std::endl;
}