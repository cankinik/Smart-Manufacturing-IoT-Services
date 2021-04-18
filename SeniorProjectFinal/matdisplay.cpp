#include "matdisplay.h"
#include <QMessageBox>
#include <QTextStream>
#include <QPainter>
#include <QTextStream>

matDisplay::matDisplay(QWidget * parent) : QLabel(parent)
{
    this->setMouseTracking(true);
    this->left_click = false;
    this->is_camera_place_mode_on = false;
    this->is_camera_placed = false;

    this->image_current = new QImage;

    this->image_current->load("/home/cankinik/Desktop/SeniorProject/EE102 PLAN.png");


    //rectangles = new std::vector<std::vector<int>>();
    this->update();

}

matDisplay::~matDisplay()
{
    delete this->image_current;
}

void matDisplay::clear_selections()
{
    this->rectangles.clear();
}

void matDisplay::activate_place_camera()
{
    this->is_camera_place_mode_on = true;
}



void matDisplay::mouseMoveEvent(QMouseEvent *mouse_event)
{
    QPoint mouse_pos = mouse_event->pos();
    if(mouse_pos.x() <= this->size().width() && mouse_pos.y() <= this->size().height()){

        if(mouse_pos.x() >= 0 && mouse_pos.y() >= 0){
            left_remain_clicked = mouse_pos;
            emit sendMousePosition(mouse_pos);
            if(left_click==true){
                this->update();
            }


        }

    }

}

void matDisplay::mousePressEvent(QMouseEvent *mouse_event)
{


    if(mouse_event->button() == Qt::LeftButton){
        left_mouse_pos_click = mouse_event->pos();
        QTextStream out(stdout);
        out << "left" << Qt::endl;
        this->left_click = true;

        if(is_camera_place_mode_on){
            is_camera_place_mode_on = false;
            is_camera_placed = true;
            camera_pos = mouse_event->pos();
            this->update();
        }
    }

}

void matDisplay::mouseReleaseEvent(QMouseEvent *mouse_event)
{
    if(mouse_event->button() == Qt::LeftButton){
        QPoint left_mouse_pos_release = mouse_event->pos();
        QTextStream out(stdout);
        out << "release" << Qt::endl;
        std::vector<int> mouse_coor = {left_mouse_pos_click.x(), left_mouse_pos_click.y(), left_mouse_pos_release.x() - left_mouse_pos_click.x(), left_mouse_pos_release.y() - left_mouse_pos_click.y() };
        rectangles.push_back(mouse_coor);
        this->left_click = false;
        this->update();

    }
}

void matDisplay::paintEvent(QPaintEvent *event)
{

    if(this->left_click == true){
        QImage scaledimg;
        scaledimg = this->image_current->scaled(this->size().width(),this->size().height(), Qt::KeepAspectRatio);

        //set size of the matDisplay according to the size of displayed floor plan size
        this->setFixedHeight(scaledimg.size().height());
        this->setFixedWidth(scaledimg.size().width());


        QPainter qPainter(this);
        qPainter.drawImage(0,0,scaledimg);

        //draw camera location if it is set
        if(is_camera_placed){
            QTextStream out(stdout);
            out << "camera" << Qt::endl;
            QPixmap pixmap2("/home/cankinik/Desktop/QTProject/SeniorProject/camera.png");

            qPainter.drawPixmap(camera_pos.x()-25,camera_pos.y()-25,50,50, pixmap2);
        }


        qPainter.setBrush(Qt::NoBrush);
        qPainter.setPen(Qt::red);

        for (auto& rect_coor : rectangles){
             qPainter.fillRect(rect_coor[0],rect_coor[1], rect_coor[2], rect_coor[3], QBrush(QColor(128, 128, 255, 128)));
        }


        qPainter.fillRect(left_mouse_pos_click.x(),left_mouse_pos_click.y(), left_remain_clicked.x() - left_mouse_pos_click.x(), left_remain_clicked.y() - left_mouse_pos_click.y(), QBrush(QColor(128, 128, 255, 128)));

    }
    else{
        QTextStream out(stdout);
        out << "Inside Paint Event" << Qt::endl;


        QImage scaledimg;
        scaledimg = this->image_current->scaled(this->size().width(),this->size().height(), Qt::KeepAspectRatio);

        //set size of the matDisplay according to the size of displayed floor plan size
        this->setFixedHeight(scaledimg.size().height());
        this->setFixedWidth(scaledimg.size().width());

        QPainter qPainter(this);
        qPainter.drawImage(0,0,scaledimg);

        //draw camera location if it is set
        if(is_camera_placed){
            QTextStream out(stdout);
            out << "camera" << Qt::endl;
            QPixmap pixmap2("/home/cankinik/Desktop/QTProject/SeniorProject/camera.png");

            qPainter.drawPixmap(camera_pos.x()-25,camera_pos.y()-25,50,50, pixmap2);
        }

        qPainter.setBrush(Qt::NoBrush);
        qPainter.setPen(Qt::red);

        for (auto& rect_coor : rectangles){
             qPainter.fillRect(rect_coor[0],rect_coor[1], rect_coor[2], rect_coor[3], QBrush(QColor(128, 128, 255, 128)));
        }


    }



}





