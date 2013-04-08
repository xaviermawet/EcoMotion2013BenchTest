#include "MainWindow.hpp"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    legendContextMenu(NULL), curveAssociatedToLegendItem(NULL),
    megasquirtDataPlot(NULL), MSPlotParser(), couplePowerPlot(NULL),
    reductionRatioPlot(NULL), distancePlot(NULL)
{
    // Display Configuration
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    // GUI configuration
    this->ui->setupUi(this);
    connect(this->ui->mainTabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(updateMenus()));
    connect(this->ui->benchTestTabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(updateMenus()));

    // Plots configuration
    this->createPlotLegendContextMenu();
    this->createCouplePowerPlotZone();
    this->createReductionRatioPlotZone();
    this->createMegasquirtDataPlotZone();
    this->createDistancePlotZone();

    // Settings configuration
    QCoreApplication::setOrganizationName("EcoMotion");
    QCoreApplication::setApplicationName("BenchTest");
    this->initSettings();
    this->readSettings();
    this->updateMenus();

    // Center the window on the screen
    this->centerOnScreen();
}

MainWindow::~MainWindow(void)
{
    qDebug() << "MainWindow début destructeur";

    delete this->ui;
    delete this->legendContextMenu;

    foreach (Plot* plot, this->plots)
        delete plot;

    qDebug() << "MainWindow Fin destructeur";
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Save the state of the mainWindow and its widgets
    this->writeSettings();

    QMainWindow::closeEvent(event);
}

void MainWindow::centerOnScreen(void)
{
    QDesktopWidget screen;

    QRect screenGeom = screen.screenGeometry(this);

    int screenCenterX = screenGeom.center().x();
    int screenCenterY = screenGeom.center().y();

    this->move(screenCenterX - width() / 2, screenCenterY - height() / 2);
}

void MainWindow::createPlotLegendContextMenu(void)
{
    // Legend actions
    this->legendContextMenu = new QMenu(this);
    this->legendContextMenu->addAction(tr("Effacer"),
                                       this, SLOT(eraseCurve()));
    this->legendContextMenu->addAction(tr("Centrer sur"),
                                       this, SLOT(centerOnCurve()));
    this->legendContextMenu->addAction(tr("Changer la couleur"),
                                       this, SLOT(changeCurveColor()));
}

void MainWindow::createMegasquirtDataPlotZone(void)
{
    this->megasquirtDataPlot = new Plot("Données du Megasquirt", this);
    this->megasquirtDataPlot->setAxisTitle(Plot::xBottom, tr("Temps (s)"));
    this->ui->megasquirtDataSplitter->addWidget(this->megasquirtDataPlot);

    // Connect plot signals to slots
    connect(this->megasquirtDataPlot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            this,  SLOT(setPlotCurveVisibile(QwtPlotItem*, bool)));
    connect(this->megasquirtDataPlot,
            SIGNAL(legendRightClicked(const QwtPlotItem*,QPoint)),
            this, SLOT(showLegendContextMenu(const QwtPlotItem*,QPoint)));

    // Settings management configuration
    this->megasquirtDataPlot->setObjectName("MegasquirtDataPlot");
    this->plots.append(this->megasquirtDataPlot);
}

void MainWindow::createCouplePowerPlotZone(void)
{
    this->couplePowerPlot = new DoubleYAxisPlot(
                "Couple - Puissance - Puissance spécifique", 0.01, this);
    this->couplePowerPlot->setAxisTitle(Plot::yLeft, tr("Couple (N.m)"));;
    this->couplePowerPlot->setAxisTitle(Plot::yRight, tr("Puissance (W)"));
    this->couplePowerPlot->setAxisTitle(Plot::xBottom,
                                        tr("Tours par minute (tr/min)"));

    // Add plot into a main window's layout
    this->ui->coupleAndPowerHLayout->addWidget(this->couplePowerPlot);

    // Connect plot signals to slots
    connect(this->couplePowerPlot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            this,  SLOT(setPlotCurveVisibile(QwtPlotItem*, bool)));
    connect(this->couplePowerPlot,
            SIGNAL(legendRightClicked(const QwtPlotItem*,QPoint)),
            this, SLOT(showLegendContextMenu(const QwtPlotItem*,QPoint)));

    // Settings management configuration
    this->couplePowerPlot->setObjectName("CouplePowerPlot");
    this->plots.append(this->couplePowerPlot);
}

void MainWindow::createReductionRatioPlotZone(void)
{
    this->reductionRatioPlot = new Plot("Rapport de réduction", this);
    this->reductionRatioPlot->setAxisTitle(Plot::yLeft,
                                           "i (rapport de réduction)");
    this->reductionRatioPlot->setAxisTitle(Plot::xBottom,
                               "Tours par minute du moteur (tr/min)");

    this->ui->reductionRatioHLayout->addWidget(this->reductionRatioPlot);

    // Connect plot signals to slots
    connect(this->reductionRatioPlot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            this,  SLOT(setPlotCurveVisibile(QwtPlotItem*, bool)));
    connect(this->reductionRatioPlot,
            SIGNAL(legendRightClicked(const QwtPlotItem*,QPoint)),
            this, SLOT(showLegendContextMenu(const QwtPlotItem*,QPoint)));

    // Settings management configuration
    this->reductionRatioPlot->setObjectName("ReductionRatioPlot");
    this->plots.append(this->reductionRatioPlot);
}

void MainWindow::createDistancePlotZone(void)
{
    this->distancePlot = new Plot(tr("Distances"), this);
    this->distancePlot->setAxisTitle(Plot::yLeft, tr("Distance (m)"));
    this->distancePlot->setAxisTitle(Plot::xBottom, tr("Temps (s)"));
    this->ui->distanceHLayout->addWidget(this->distancePlot);

    // Connect plot signals to slots
    connect(this->distancePlot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            this,  SLOT(setPlotCurveVisibile(QwtPlotItem*, bool)));
    connect(this->distancePlot,
            SIGNAL(legendRightClicked(const QwtPlotItem*,QPoint)),
            this, SLOT(showLegendContextMenu(const QwtPlotItem*,QPoint)));

    // Settings management configuration
    this->distancePlot->setObjectName("DistancePlot");
    this->plots.append(this->distancePlot);
}

Plot* MainWindow::currentPlot(void) const
{
    switch (this->ui->mainTabWidget->currentIndex())
    {
    case TAB_BENCH_TEST:
        switch (this->ui->benchTestTabWidget->currentIndex())
        {
            case TAB_COUPLE_AND_POWER:
                return this->couplePowerPlot;
            case TAB_REDUCTION_RATIO:
                return this->reductionRatioPlot;
            case TAB_DISTANCE:
                return this->distancePlot;
            default:
                return NULL;
        }
    case TAB_MEGASQUIRT_DATA:
        return this->megasquirtDataPlot;
    default:
        return NULL;
    }
}

void MainWindow::updateMenus(void)
{
    // Get the current plot
    Plot* plot = this->currentPlot();
    if (!plot) return;

    // Update menu edit actions
    this->ui->actionShowGrid->setChecked(plot->isGridVisible());
    this->ui->actionShowCrossLine->setChecked(plot->isCrossLineVisible());
    this->ui->actionShowLabelPosition->setChecked(
                plot->isLabelPositionVisible());

    // Update menu file actions
    int currentMainTabWidgetIndex = this->ui->mainTabWidget->currentIndex();

    this->ui->actionImportData->setVisible(
                currentMainTabWidgetIndex == TAB_BENCH_TEST);
    this->ui->actionDatToCSV->setVisible(
                currentMainTabWidgetIndex == TAB_MEGASQUIRT_DATA);
    this->ui->actionLoadCSV->setVisible(
                currentMainTabWidgetIndex == TAB_MEGASQUIRT_DATA);

    // Update menu configure actions
    this->ui->menuConfigure->menuAction()->setVisible(
                currentMainTabWidgetIndex == TAB_BENCH_TEST);
}

void MainWindow::readSettings(void)
{
    QSettings settings;

    // Restore plots settings
    foreach (Plot* plot, this->plots)
    {
        settings.beginGroup(plot->objectName());
        plot->setGridVisible(
                    settings.value("isGridVisible", true).toBool());
        plot->setCrossLineVisible(
                    settings.value("isCrossLineVisible", false).toBool());
        plot->setLabelPositionVisible(
                    settings.value("isLabelPositionVisible", true).toBool());
        settings.endGroup();
    }

    // Restore MainWindow settings
    settings.beginGroup("MainWindow");

    /* Contourne le bug non résolu par Qt de la restauration de la géométrie
     * d'une fenetre maximisée alors qu'elle est déjà maximisée */
    if (settings.value("isMaximized", true).toBool())
        this->showMaximized();
    else
        this->restoreGeometry(settings.value("geometry").toByteArray());

    this->ui->mainTabWidget->setCurrentIndex(
                settings.value("mainTabWidgetCurrentIndex", 0).toInt());
    this->ui->benchTestTabWidget->setCurrentIndex(
                settings.value("benchTestTabWidgetCurrentIndex", 0).toInt());
    this->ui->megasquirtDataSplitter->restoreState(
                settings.value("megasquirtDataSplitter").toByteArray());

    settings.endGroup();
}

void MainWindow::writeSettings(void) const
{
    QSettings settings;

    // Save plots settings
    foreach (Plot* plot, this->plots)
    {
        settings.beginGroup(plot->objectName());
        settings.setValue("isGridVisible",
                          plot->isGridVisible());
        settings.setValue("isCrossLineVisible",
                          plot->isCrossLineVisible());
        settings.setValue("isLabelPositionVisible",
                          plot->isLabelPositionVisible());
        settings.endGroup();
    }

    // Restore MainWindow settings
    settings.beginGroup("MainWindow");
    settings.setValue("isMaximized", this->isMaximized());
    settings.setValue("geometry", this->saveGeometry());
    settings.setValue("mainTabWidgetCurrentIndex",
                      this->ui->mainTabWidget->currentIndex());
    settings.setValue("benchTestTabWidgetCurrentIndex",
                      this->ui->benchTestTabWidget->currentIndex());
    settings.setValue("megasquirtDataSplitter",
                      this->ui->megasquirtDataSplitter->saveState());
    settings.endGroup();
}

void MainWindow::initSettings(void) const
{
    QSettings settings;

    settings.beginGroup("files");

    if(!settings.contains(KEY_INERTIE))
        settings.setValue(KEY_INERTIE, "Inertie.csv");

    if(!settings.contains(KEY_PROTOWHEEL))
        settings.setValue(KEY_PROTOWHEEL, "ProtoWheel.csv");

    if(!settings.contains(KEY_MEGASQUIRT_DAT))
        settings.setValue(KEY_MEGASQUIRT_DAT, "Megasquirt.dat");

    if(!settings.contains(KEY_MEGASQUIRT_CSV))
        settings.setValue(KEY_MEGASQUIRT_CSV, "Megasquirt.csv");

    settings.endGroup();
}

void MainWindow::checkFolderContent(const QDir &MSDir) const
{
    if (!MSDir.exists())
        throw QException(tr("Le dossier n'existe pas"));

    // Get all file names from settings
    QSettings settings;
    settings.beginGroup("files");

    QStringList fileNames;
    fileNames << settings.value(KEY_INERTIE).toString()
              << settings.value(KEY_PROTOWHEEL).toString()
              << settings.value(KEY_MEGASQUIRT_DAT).toString();

    settings.endGroup();

    // Check if the folder content all the files
    foreach (QString filename, fileNames)
        if(!MSDir.exists(filename))
            throw QException(tr("Fichier ") + filename + tr(" inexistant"));
}

void MainWindow::createCoupleAndPowerCurves(const QString& inertieCSVFilename,
                                            const QString& msCSVFilename)
{
    // Open inertie file that contains time values
    QFile inertieFile(inertieCSVFilename);
    if (!inertieFile.open(QIODevice::ReadOnly | QIODevice::Text))
        throw QException(QObject::tr("Impossible d'ouvrir le fichier ")
                         + inertieCSVFilename);

    // Read the file line-by-line and covert time value into seconds
    QList<double> inertieTimes;
    while(!inertieFile.atEnd())
        inertieTimes.append(QString(inertieFile.readLine()).toDouble() / 1000000);
    inertieFile.close();

    // Remove wrong values (first ones)
    for (int i(0); i < 6; ++i)
        inertieTimes.pop_front();

    // We need at least 3 values of time to calculate the couple
    if (inertieTimes.count() < 3)
        throw QException(QObject::tr("Le fichier ") + inertieCSVFilename +
                         QObject::tr(" ne contient pas assez de valeurs"));

    // Create couple and power curves
    const double PI = 3.141592653589793;
    const double Jdelta = 0.22; // FIXME : demander à l'utilisateur de rentrer la valeur

    double angularSpeed_a, angularSpeed_b;
    double ta_average, tb_average;
    double angularAcceleration;
    double couple, power;

    QVector<QPointF> powerPoints;
    QVector<QPointF> couplePoints;

    /* ---------------------------------------------------------------------- *
     *                           ωa = 2π / (t2 - t1)                          *
     * ---------------------------------------------------------------------- *
     * ωa = Première vitesse angulaire (rad/s)                                *
     * Π  = Pi, constante qui vaut 3.141592653589793...                       *
     * tx = temps à l'instant x (s) ave t2 > t1                               *
     * ---------------------------------------------------------------------- */

    angularSpeed_b = (2 * PI) / (inertieTimes.at(1) - inertieTimes.at(0));

    for (int i(2); i < inertieTimes.count(); ++i)
    {
        angularSpeed_a = angularSpeed_b;

    /* ---------------------------------------------------------------------- *
     *                           ωb = 2π / (t3 - t2)                          *
     * ---------------------------------------------------------------------- *
     * ωb = Deuxième vitesse angulaire (rad/s)                                *
     * Π  = Pi, constante qui vaut 3.141592653589793...                       *
     * tx = temps à l'instant x (s) ave t2 > t1                               *
     * ---------------------------------------------------------------------- */

        angularSpeed_b = (2 * PI) / (inertieTimes.at(i) - inertieTimes.at(i-1));

    /* ---------------------------------------------------------------------- *
     *                        ta_moyen = (t1 + t2) / 2                        *
     *                        tb_moyen = (t2 + t3) / 2                        *
     *                   α = (ωb - ωa) / (tb_moyen - ta_moyen)                *
     * ---------------------------------------------------------------------- *
     * α  = Accélération angulaire (rad/s²)                                   *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * tx = temps x (s)                                                       *
     * ---------------------------------------------------------------------- */

        //accAngulaire = (wb - wa) / tb;

        ta_average = (inertieTimes.at(i - 2) + inertieTimes.at(i - 1)) / 2;
        tb_average = (inertieTimes.at(i - 1) + inertieTimes.at(i)) / 2;

        angularAcceleration = (angularSpeed_b - angularSpeed_a)
                                              /
                                  (tb_average - ta_average);

    /* ---------------------------------------------------------------------- *
     *                              C = JΔ * α                                *
     * ---------------------------------------------------------------------- *
     * C  = Couple (N.m)                                                      *
     * JΔ = moment d'inertie (kg.m²)                                          *
     * α  = Accélération angulaire (rad/s²)                                   *
     * ---------------------------------------------------------------------- */

        couple = Jdelta * angularAcceleration;

    /* ---------------------------------------------------------------------- *
     *                               P = C * ωb                               *
     * ---------------------------------------------------------------------- *
     * P  = Puissance (Watts)                                                 *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * ---------------------------------------------------------------------- */

        power = couple * angularSpeed_b;

    /* ---------------------------------------------------------------------- *
     *                   Calcul de la masse de carburant                      *
     * ---------------------------------------------------------------------- */


    /* ---------------------------------------------------------------------- *
     *               ωx = (π * Nx) / 30  <=> Nx = (30 * ωx) / π               *
     * ---------------------------------------------------------------------- *
     * ω = Vitesse angulaire (rad/s)                                          *
     * Π = Pi, constante qui vaut 3.141592653589793...                        *
     * N = tours par minute == RPM (tours/minute)                             *
     * ---------------------------------------------------------------------- */

            double rpm_b = (30 * angularSpeed_b) / PI;

            // create curves coordinates
            powerPoints.append(QPointF(rpm_b, power));
            couplePoints.append(QPointF(rpm_b, couple));
    }

    // Create power curve
    QwtPointSeriesData* powerSerieData = new QwtPointSeriesData(powerPoints);
    PlotCurve* powerCurve = new PlotCurve(tr("Puissance"), QPen(Qt::darkBlue)); // TODO : ajouter le nom de l'essai (par défaut, le nom du dossier)
    powerCurve->setData(powerSerieData);
    powerCurve->setAxes(Plot::xTop, Plot::yRight);
    powerCurve->attach(this->couplePowerPlot);
    this->setPlotCurveVisibile(powerCurve, true);

    // Create couple curve
    QwtPointSeriesData* coupleSerieData = new QwtPointSeriesData(couplePoints);
    PlotCurve* coupleCurve = new PlotCurve(tr("Couple"), QPen(Qt::darkRed)); // TODO : ajouter le nom de l'essai (par défaut, le nom du dossier)
    coupleCurve->setData(coupleSerieData);
    coupleCurve->attach(this->couplePowerPlot);
    this->setPlotCurveVisibile(coupleCurve, true);

    // Zoom on the biggest curve
    this->couplePowerPlot->zoom(powerCurve);

    // Détermination de l'équation correspondant le plus à l'approximation polynomiales
//    QVector<QPointF> points;
//    points.append(QPointF(0, 36));
//    points.append(QPointF(20, 35.2));
//    points.append(QPointF(40, 33.2));
//    points.append(QPointF(60, 30.1));
//    points.append(QPointF(80, 25.8));
//    points.append(QPointF(100, 20.8));
    this->leastSqrRegression(powerPoints, powerCurve->title().text(),
                             Plot::xTop, Plot::yRight);
    this->leastSqrRegression(couplePoints, coupleCurve->title().text(),
                             Plot::xBottom, Plot::yLeft);
}

void MainWindow::leastSqrRegression(const QVector<QPointF> &points,
                                    QString const& baseName,
                                    QwtPlot::Axis xAxis, QwtPlot::Axis yAxis)
{
   if (points.count() == 0)
   {
       qDebug() << "Aucune données à traiter";
       return;
   }

   // double  = 8  octets = 64  bits
   // decimal = 16 octets = 128 bits

   double SUMx4(0);  // sum of x^4
   double SUMx3(0);  // sum of x³
   double SUMx2(0);  // sum of x²
   double SUMx(0);   // sum of x
   double SUMx2y(0); // sum of x²*y
   double SUMxy(0);  // sum of x*y
   double SUMy(0);   // sum of y

   double a(0);
   double b(0);
   double c(0);

   // y = ax² + bx + c

   foreach (QPointF point, points)
   {
       if (point.y() < 0.0)
           continue;
       SUMx4  += qPow(point.x(), 4);
       SUMx3  += qPow(point.x(), 3);
       SUMx2  += qPow(point.x(), 2);
       SUMx   += point.x();
       SUMx2y += qPow(point.x(), 2) * point.y();
       SUMxy  += point.x() * point.y();
       SUMy   += point.y();
   }

   qDebug() << "x4 = " << SUMx4;
   qDebug() << "x3 = " << SUMx3;
   qDebug() << "x2 = " << SUMx2;
   qDebug() << "x = " << SUMx;
   qDebug() << "x²*y = " << SUMx2y;
   qDebug() << "x*y = " << SUMxy;
   qDebug() << "y = " << SUMy;

   a = ((points.count()*SUMx2y-SUMx2*SUMy)*(points.count()*SUMx2-qPow(SUMx, 2))-(points.count()*SUMxy-SUMx*SUMy)*(points.count()*SUMx3-SUMx2*SUMx))
           /
   ((points.count()*SUMx4-qPow(SUMx2, 2))*(points.count()*SUMx2-qPow(SUMx, 2))-qPow(SUMx2*SUMx-points.count()*SUMx3, 2));

   qDebug() << "a = " << a;

   b = (a*(SUMx2*SUMx-points.count()*SUMx3)+points.count()*SUMxy-SUMx*SUMy)/(points.count()*SUMx2-qPow(SUMx, 2));

   qDebug() << "b = " << b;

   c = (-a*SUMx2-b*SUMx+SUMy)/points.count();

   qDebug() << "c = " << c;

   QVector<QPointF> approxiPoints;
   foreach (QPointF point, points)
   {
       // y = a*x² + b*x + c
       approxiPoints.append(QPointF(point.x(), a*qPow(point.x(), 2) + b*point.x() + c));
   }

   // Création de la courbe
   QwtPointSeriesData* approxiSerieData = new QwtPointSeriesData(approxiPoints);
   PlotCurve* approxiCurve = new PlotCurve(tr("approxi ") + baseName, QPen(Qt::darkMagenta)); // TODO : ajouter le nom de l'essai (par défaut, le nom du dossier)
   approxiCurve->setAxes(xAxis, yAxis);
   approxiCurve->setData(approxiSerieData);
   approxiCurve->attach(this->couplePowerPlot);
   this->setPlotCurveVisibile(approxiCurve, true);
}

void MainWindow::createCoupleAndPowerCurves_old(
        QString const& megasquirtCSVFilename)
{
    QCSVParser MSFile(megasquirtCSVFilename, ';', QString::KeepEmptyParts);
    QCSVColumn times = MSFile["times"];
    QCSVColumn rpm   = MSFile["rpm"];

    QVector<QPointF> couplePoints;
    QVector<QPointF> powerPoints;

    double w1, w2, wIntermediate;
    double accAngulaire;
    double couple, puissance;
    const double PI = 3.141592653589793;
    const double Jdelta = 0.22; // FIXME : demander à l'utilisateur de rentrer la valeur

    int lastDataIndice(0); // Used for futur data filtering.

    w2 = (PI * rpm.at(0).toDouble()) / 30;

    for (int i(1); i < times.count(); ++i)
    {
        w1 = w2;

    /* ---------------------------------------------------------------------- *
     *                           ωx = (π * Nx) / 30                           *
     * ---------------------------------------------------------------------- *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * Π  = Pi, constante qui vaut 3.141592653589793...                       *
     * Nx = tours par minute à l'instant x == RPM (tours/minute)              *
     * ---------------------------------------------------------------------- */

        w2 = (PI * rpm.at(i).toDouble()) / 30;

        qDebug() << "w1 = " << w1;
        qDebug() << "w2 = " << w2;

    /* ---------------------------------------------------------------------- *
     *                        α = (ω2 - ω1) / (t2 - t1)                       *
     * ---------------------------------------------------------------------- *
     * α  = Accélération angulaire (rad/s²)                                   *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * tx = temps x (s)                                                       *
     * ---------------------------------------------------------------------- */

        accAngulaire =                    (w2 - w1)
                                              /
     ((times.at(i).toDouble() - times.at(lastDataIndice).toDouble()) / 1000000);
        // times values are microseconds

        qDebug() << "t2 = " << times.at(i).toDouble() << " microsecondes";
        qDebug() << "t1 = " << times.at(lastDataIndice).toDouble() << " microsecondes";
        qDebug() << "Δt (t2 - t1) = " << ((times.at(i).toDouble() - times.at(lastDataIndice).toDouble()) / 1000000);
        qDebug() << "α = " << accAngulaire;

    /* ---------------------------------------------------------------------- *
     *                              C = JΔ * α                                *
     * ---------------------------------------------------------------------- *
     * C  = Couple (N.m)                                                      *
     * JΔ = moment d'inertie (kg.m²)                                          *
     * α  = Accélération angulaire (rad/s²)                                   *
     * ---------------------------------------------------------------------- */

        couple = Jdelta * accAngulaire;

        // Ajout du point à la liste des points de la courbe du couple
        couplePoints.append(QPointF(rpm.at(i).toDouble(), couple));
        qDebug() << "couple (rpm, couple) = " << couplePoints.last();

    /* ---------------------------------------------------------------------- *
     *                        P = C * ((ω1 + ω2) / 2)                         *
     * ---------------------------------------------------------------------- *
     * P  = Puissance (Watts)                                                 *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * ---------------------------------------------------------------------- */

        wIntermediate = (w1 + w2) / 2;
        qDebug() << "(w1 + w2) / 2 = " << wIntermediate;

        puissance = couple * wIntermediate;

    /* ---------------------------------------------------------------------- *
     *               ωx = (π * Nx) / 30  <=> Nx = (30 * ωx) / π               *
     * ---------------------------------------------------------------------- *
     * ωx = Vitesse angulaire à l'instant x (rad/s)                           *
     * Π  = Pi, constante qui vaut 3.141592653589793...                       *
     * Nx = tours par minute à l'instant x == RPM (tours/minute)              *
     * ---------------------------------------------------------------------- */
        powerPoints.append(QPointF((30 * wIntermediate) / PI, puissance));
        qDebug() << "puissance (rpm, puissance) = " << powerPoints.last();

        lastDataIndice = i;
    }

    // Création de la courbe du couple
    QwtPointSeriesData* coupleSerieData = new QwtPointSeriesData(couplePoints);
    PlotCurve* coupleCurve = new PlotCurve(tr("Couple"), QPen(Qt::darkRed)); // TODO : ajouter le nom de l'essai (par défaut, le nom du dossier)
    coupleCurve->setData(coupleSerieData);
    coupleCurve->attach(this->couplePowerPlot);
    this->setPlotCurveVisibile(coupleCurve, true);

    // Création de la courbe de la puissance
    QwtPointSeriesData* powerSerieData = new QwtPointSeriesData(powerPoints);
    PlotCurve* powerCurve = new PlotCurve(tr("Puissance"), QPen(Qt::darkBlue)); // TODO : ajouter le nom de l'essai (par défaut, le nom du dossier)
    powerCurve->setData(powerSerieData);
    powerCurve->setAxes(Plot::xBottom, Plot::yRight);
    powerCurve->attach(this->couplePowerPlot);
    this->setPlotCurveVisibile(powerCurve, true);

    qDebug() << "Fin de la création des courbes ...";
}

void MainWindow::on_actionImportData_triggered(void)
{
    QString dirPath = QFileDialog::getExistingDirectory(
                      this, tr("Sélectionnez le dossier contenant "
                      "les données du Megasquirt"), QDir::homePath());

    if (dirPath.isEmpty())
        return;

    QSettings settings;
    settings.beginGroup("files");

    try
    {
        // Check the folder content
        QDir MSDir(dirPath);
        this->checkFolderContent(MSDir);

        // Set import parameters
        MSDataParameterDialog importParamDial(MSDir.dirName(), this);
        if (importParamDial.exec() == QDialog::Rejected)
            return;

        // Remove oldest megasquirt csv file if exists
        QString msCSVFilePath = MSDir.filePath(
                    settings.value(KEY_MEGASQUIRT_CSV).toString());
        QFile msCSVFile(msCSVFilePath);
        if (msCSVFile.exists())
            msCSVFile.remove();

        // Generate csv file by extracting needed data from dat file
        QStringList megasquirtParameters;
        megasquirtParameters << "pulseWidth1" << "rpm" << "batteryVoltage";

        MSManager megasquirtManager;
        megasquirtManager.datToCSV
                (
                  MSDir.filePath(settings.value(KEY_MEGASQUIRT_DAT).toString()),
                  msCSVFilePath,
                  megasquirtParameters
                );

        // Create couple and power curves
        QString inertieFilePath = MSDir.filePath(
                    settings.value(KEY_INERTIE).toString());
        this->createCoupleAndPowerCurves(inertieFilePath, msCSVFilePath);
    }
    catch(QException const& ex)
    {
        QMessageBox::warning(this, tr("Importation annulée"), ex.what());
    }

    settings.endGroup();
}

void MainWindow::on_actionQuit_triggered(void)
{
    // Save the state of the mainWindow and its widgets
    this->writeSettings();

    // Close the main window
    qApp->quit();
}

void MainWindow::on_addCurvePushButton_clicked(void)
{
    QString curveName = this->ui->paramMegasquirtComboBox->currentText();

    try
    {
        if (curveName.isEmpty())
            throw QException(tr("Aucune courbe sélectionnée"));

        qDebug() << "Courbe a afficher : " << curveName;

        // Récupération du temps exprimé en microsecondes
        QCSVRow secondes = this->MSPlotParser["times"];

        // Récupération de la colonne de données a représenter par une courbe
        QCSVRow data     = this->MSPlotParser[curveName];

        // Création de la liste de points de la courbe
        QVector<QPointF> vect;
        for (int i(0); i < data.count(); ++i)
        {
            qreal xTps = secondes.at(i).toDouble() / 1000000;
            qreal yData = data.at(i).toDouble();
            vect.append(QPointF(xTps, yData));
        }

        QwtPointSeriesData* serieData = new QwtPointSeriesData(vect);

        // Create a new curves
        QwtPlotCurve* curve = new QwtPlotCurve(curveName);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);
        curve->setItemAttribute(QwtPlotItem::Legend);
        curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
        curve->setPen(QPen(Qt::darkRed, 1));
        curve->setData(serieData);
        curve->attach(this->megasquirtDataPlot);

        this->setPlotCurveVisibile(curve, true);
    }
    catch (QException const& ex)
    {
        QMessageBox::information(this, tr("Ajout de la courbe annulé"),
                                 ex.what());
    }
}

void MainWindow::on_actionIncreaseAccuracy_triggered(void)
{
    // Get the current plot
    Plot* plot = this->currentPlot();

    // Set the maximum number of major scale intervals for a specified axis
    plot->setAxisMaxMajor(QwtPlot::yLeft,
                          plot->axisMaxMajor(QwtPlot::yLeft) + 1);
    plot->setAxisMaxMajor(QwtPlot::yRight,
                          plot->axisMaxMajor(QwtPlot::yRight) + 1);
    plot->setAxisMaxMajor(QwtPlot::xBottom,
                          plot->axisMaxMajor(QwtPlot::xBottom) + 1);
}

void MainWindow::on_actionReduceAccuracy_triggered(void)
{
    // Get the current plot
    Plot* plot = this->currentPlot();

    // Set the maximum number of major scale intervals for a specified axis
    plot->setAxisMaxMajor(QwtPlot::yLeft,
                          plot->axisMaxMajor(QwtPlot::yLeft) - 1);
    plot->setAxisMaxMajor(QwtPlot::yRight,
                          plot->axisMaxMajor(QwtPlot::yRight) - 1);
    plot->setAxisMaxMajor(QwtPlot::xBottom,
                          plot->axisMaxMajor(QwtPlot::xBottom) - 1);
}

void MainWindow::on_actionShowGrid_triggered(bool visible)
{
    this->currentPlot()->setGridVisible(visible);
}

void MainWindow::on_actionShowLabelPosition_triggered(bool visible)
{
    this->currentPlot()->setLabelPositionVisible(visible);
}

void MainWindow::on_actionShowCrossLine_triggered(bool visible)
{
    this->currentPlot()->setCrossLineVisible(visible);
}

void MainWindow::on_actionLoadCSV_triggered(void)
{
    /* ---------------------------------------------------------------------- *
     *                         Récupérer le fichier CSV                       *
     * ---------------------------------------------------------------------- *
     * Devra changer --> sélection d'un dossier qui contient tous les         *
     * fichiers nécéssaires                                                   *
     * ---------------------------------------------------------------------- */
    QString filename = QFileDialog::getOpenFileName
            (
                this, tr("Megasquirt - Ouvrir un fichier CSV"),
                QDir::homePath(), tr("Fichiers CSV (*.csv)")
            );

    if (filename.isEmpty())
        return;

    try
    {
        // Load information from the CSV file
        this->MSPlotParser.parse(filename, ';', QString::SkipEmptyParts);

        // Récupérer les noms des colonnes du fichier csv
        this->ui->paramMegasquirtComboBox->clear();
        this->ui->paramMegasquirtComboBox->addItems(this->MSPlotParser.headersList());
    }
    catch (QException const& ex)
    {
        QMessageBox::warning(this, tr("Importation annulée"), ex.what());
    }
}

void MainWindow::on_actionDatToCSV_triggered(void)
{
    try
    {
        /* Get dat file name */
        QString datFileName = QFileDialog::getOpenFileName(
                    this, tr("Selectionné le fichier de données du Megasquirt"),
                    QDir::homePath(), tr("Fichier de données (*.dat)"));

        MSFileConverterDialog dialog(datFileName, this);
        dialog.exec();
    }
    catch(QException const& ex)
    {
        QMessageBox::warning(this, tr("Conversion annulée"), ex.what());
    }
}

void MainWindow::on_actionConfigureMegasquirtFileName_triggered(void)
{
    MSFileParameterDialog dial(this);
    dial.exec();
}

void MainWindow::on_actionExportToPDF_triggered(void)
{
    QString pdfFile = QFileDialog::getSaveFileName(
                this, tr("Sauvegarder le graphique"), QDir::homePath(),
                tr("Portable Document Format (*.pdf)"));

    if (pdfFile.isNull() || pdfFile.isEmpty())
        return; // User cancel the previous dialog

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
    renderer.renderDocument(this->currentPlot(), pdfFile,
                            this->currentPlot()->size());
}

void MainWindow::eraseCurve(void)
{
    // if no curve associated to the legend item. This shouldn't happen!
    if (this->curveAssociatedToLegendItem == NULL)
        return;

    // Delete the curve associated to the legend item
    this->curveAssociatedToLegendItem->detach();
    delete this->curveAssociatedToLegendItem;
    this->curveAssociatedToLegendItem = NULL;

    // update the plot
    this->currentPlot()->replot();
}

void MainWindow::centerOnCurve(void)
{
    // if no curve associated to the legend item. This shouldn't happen!
    if (this->curveAssociatedToLegendItem == NULL)
        return;

    this->currentPlot()->zoom(this->curveAssociatedToLegendItem);
}

void MainWindow::changeCurveColor(void)
{
    // if no curve associated to the legend item. This shouldn't happen!
    if (this->curveAssociatedToLegendItem == NULL)
        return;

    // Select a new color
    QColor newColor = QColorDialog::getColor(
                this->curveAssociatedToLegendItem->pen().color(), this,
                tr("Choisir une nouvelle couleur pour la courbe"));

    // If the user cancels the dialog, an invalid color is returned
    if (newColor.isValid())
        this->curveAssociatedToLegendItem->setPen(QPen(newColor));
}

void MainWindow::setPlotCurveVisibile(QwtPlotItem* item, bool visible)
{
    item->setVisible(visible);
    QWidget* w = item->plot()->legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(visible);

    item->plot()->replot();
}

void MainWindow::showLegendContextMenu(const QwtPlotItem* item,
                                       const QPoint& pos)
{
    // Save the plot curve associated to the legend item
    this->curveAssociatedToLegendItem = (PlotCurve*) item;
    if(!this->curveAssociatedToLegendItem)
        return;

    // Display custom contextual menu
    this->legendContextMenu->exec(pos);
}
