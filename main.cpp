#include "krypto.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QRandomGenerator>
#include <QTextCodec>

// Все вершины
struct vertex{
    int     num;
    qint64  rand;
    int    color;
    qint64  c;
    qint64  d;
    qint64  n;
    qint64  z;
};
// Вершины для Боба
struct vert_for_bob{
    int     num;
    qint64  c;
    qint64  d;
    qint64  n;
    qint64  z;
};
// Записываем индексы выбранных Бобом ребер
QVector<int> rand_of_bob(int n_edges){
    QVector<int> vec;
    int n;
    if(n_edges > 4){
        n = QRandomGenerator::global()->bounded(2, n_edges / 2);
    } else{
        n = 2;
    }
    for(int i = 0; i < n; i++){
        vec << QRandomGenerator::global()->bounded(n_edges);
    }
    return vec;
}
void shift(int mas[], int n)
{
    // Смещаем цвета на 1-у или 2-е позиции
    for(int i = 0, j = n; i < 3; i++){
        if(j == 3) j = 0;
        mas[i] = j++;
    }
}

QString show_color(qint64 col){
    QStringList color = {"Красный", "Зеленый", "Синий"};
    return color[col];
}

void show_data_vert(QVector<vertex> vec){
    for(auto vert : vec){
        qDebug() << "num = " << vert.num;
        qDebug() << "rand = " << vert.rand;
        qDebug() << "c = " << vert.c << "d = " << vert.d << "n = " << vert.n << "z = " << vert.z;
    }
}

void show_color_vert(QVector<vertex> vec, int col_mas[], bool flg = false)
{
    for(auto vert : vec){
        qDebug() << "num = " << vert.num;
        if(flg){
            qDebug() << "color = " << show_color(col_mas[vert.color]);
        } else{
            qDebug() << "color = " << show_color(vert.color);
        }
    }
}

void gen_cd(qint64 p, qint64 &c, qint64 &d)
{
    qint64 gcd_mass[3];
    do{
        c = QRandomGenerator::global()->bounded(1, (int)p);
    }while(GCD_two(c, p) != 1);
    gcd(p,c,gcd_mass);
    d = gcd_mass[2];
}

void bob(QVector<vert_for_bob> vfb, QVector<int> index_edges, QVector<QVector <int>> edges){
    int edge;
    qDebug("\n");
    qint64 z1, z2;
    for(int i = 0; i < index_edges.size(); i++){
        edge = index_edges[i];
        z1 = exp_mod(vfb[edges[edge][0]].z, vfb[edges[edge][0]].c, vfb[edges[edge][0]].n) & 3;
        z2 = exp_mod(vfb[edges[edge][1]].z, vfb[edges[edge][1]].c, vfb[edges[edge][1]].n) & 3;

        if(vfb[edges[edge][0]].num == -1 || vfb[edges[edge][1]].num == -1){ qDebug("Все плохо!"); }
        qDebug() << vfb[edges[edge][0]].num << " <-(" << index_edges[i] << ")-> " << vfb[edges[edge][1]].num;
        // Проверяем смежные вершины на различие цветов
        //        qDebug() << show_color(z1) << " = (" << index_edges[i] << ") = " << show_color(z2);
        if(z1 != z2){
            qDebug() << "Ребро " << index_edges[i] << " - в порядке!";
        }else{
            qDebug() << "Ребро " << index_edges[i] << " - не в порядке";
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifdef Q_OS_WIN32
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 866"));
#endif
    int n_vert = 0, n_edge = 0;
    QVector<vertex> vec_vert;
    QVector<vert_for_bob> vec_vfb;
    vertex vert;
    vert_for_bob vfb;
    QVector<QVector <int>> vec_edges;
    // Открываем файл
    QFile file(":/test.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug("Все плохо =(");
    }
    QTextStream in(&file);
    QString line;
    QStringList row;
    // Считываем файл в строку
    while (!in.atEnd()) {
        line += in.readLine() + "\n";
    }
    file.close();
    QStringList list = line.split("\n");
    QVector<int> numbers;
    // Считываем данные из строки
    for(int i = 0, id_vert = 0; i < list.size(); i++){
        row = list[i].split(" ");
        if(row[0] == "n"){
            n_vert = row[1].toInt();
            n_edge = row[2].toInt();
        } else if(row[0] == "e"){
            numbers << row[1].toInt();
            numbers << row[2].toInt();
            vec_edges << numbers;
            numbers.clear();
        } else if(row[0] == "c"){
            vert.num = id_vert++;
            vert.color = row[1].toInt();
            vec_vert << vert;
        }
    }
    // Перемешиваем цвета для боба
    int colors[] = {0, 1, 2};
    shift(colors, QRandomGenerator::global()->bounded(1, 3));
    qint64 p, q, fi;
    // Генерируем данные для вершин
    for (int i = 0; i < vec_vert.size(); i++) {
        // Заделали большое число с цветом
        vec_vert[i].rand = QRandomGenerator::global()->bounded(1 << 16, 1 << 17);
        vec_vert[i].rand = vec_vert[i].rand >> 2;
        vec_vert[i].rand = vec_vert[i].rand << 2;
        vec_vert[i].rand += colors[vec_vert[i].color];
        // gen_rsa
        p = gen_prime_num(1 << 10, 1 << 11);
        q = gen_prime_num(1 << 10, 1 << 11);
        vec_vert[i].n = p * q;
        fi = (p - 1) * (q - 1);
        gen_cd(fi, vec_vert[i].c, vec_vert[i].d);
        vec_vert[i].z = exp_mod(vec_vert[i].rand, vec_vert[i].d, vec_vert[i].n);
        vec_vert[i].num = i;
    }
    qDebug("Данные вершин:");
    show_data_vert(vec_vert);
    qDebug("\nЦвета вершин у Алисы:");
    show_color_vert(vec_vert, colors);
    qDebug("\nЦвета вершин у Боба:");
    show_color_vert(vec_vert, colors, 1);
    // Боб выбирает рандомные ребра
    QVector<int> index_edges = rand_of_bob(vec_edges.size());
    QVector<bool> fsfs(n_vert, false);
    // Запомнили, какие нужны вершины
    for(int i = 0; i < index_edges.size(); i++){
        if(!fsfs[vec_edges[index_edges[i]][0]]){
            fsfs[vec_edges[index_edges[i]][0]] = true;
        }
        if(!fsfs[vec_edges[index_edges[i]][1]]){
            fsfs[vec_edges[index_edges[i]][1]] = true;
        }
    }
    // Заполняем данные нужных бобу вершин
    for(int i = 0; i < vec_vert.size(); i++){
        if(fsfs[i]){
            vfb.c = vec_vert[i].c;
            vfb.d = vec_vert[i].d;
            vfb.n = vec_vert[i].n;
            vfb.z = vec_vert[i].z;
            vfb.num = vec_vert[i].num;
        } else{
            vfb.c = 0;
            vfb.d = 0;
            vfb.n = 0;
            vfb.z = 0;
            vfb.num = -1;
        }
        vec_vfb << vfb;
    }
    // Отправили данные бобу
    bob(vec_vfb, index_edges, vec_edges);
    return a.exec();
}
