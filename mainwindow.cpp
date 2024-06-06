#include "mainwindow.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include <QTreeView>
#include <QGridLayout>
#include <QGraphicsView>
#include <QLabel>
#include <QComboBox>
#include <QSplitter>
#include <QPushButton>
#include <QHeaderView>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QDateTime>
#include <QSlider>
#include <QGroupBox>
#include <QTimer>
#include <QUndoStack>
#include <QTabWidget>
#include <QDir>
#include <QButtonGroup>
#include <QToolBar>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QAction>
#include <QStatusBar>
#include <QDateTimeEdit>
#include <QLCDNumber>
#include <QMediaPlayer>
#include <QSpinBox>


#include "data.h"
#include "mytaskorggraphicsscene.h"
#include "mymapgraphicsscene.h"
#include "mystandarditemmodel.h"
#include "zoomeventfilter.h"
#include "unit.h"
#include "treedelegate.h"
#include "unitdetail.h"
#include "commands.h"
#include "network.h"
#include "player.h"
#include "order.h"
#include "VectorEditor/vepolylinesettings.h"
#include "VectorEditor/veshapesettings.h"
#include "VectorEditor/vepolyline.h"
#include "VectorEditor/verectangle.h"
#include "VectorEditor/veellipse.h"

#include "modeltest.h"

MainWindow::MainWindow(bool isAdmin, QString sessionName, bool isObserver, QWidget *parent) :
    QMainWindow(parent),
    mIsAdmin(isAdmin),
    mIsObserver(isObserver),
    mSessionName(sessionName)
{
    if(mIsAdmin)
        qDebug() << "SESSION IS ADMIN";

    // Data structures
    mData = new Data(this);
    //    mModel = new MyStandardItemModel(mData, this);
    mTaskOrgScene = new MyTaskOrgGraphicsScene(mData, this);
    mMapScene = new MyMapGraphicsScene(mData, mIsAdmin, this);
    //    mMapScene->setSessionName(mSessionName);

    //    QStringList headers;
    //    headers << tr("Icon") << tr("Name") << tr("Add") << tr("Delete");
    //    mTreeModel = new TreeModel(headers, this);

    mModel = new MyStandardItemModel(mData);
    //    new ModelTest(mModel, this);

    mTimer = new QTimer(this);

    mMediaPlayer = new QMediaPlayer(this);


    setupActions();
    setupGUI();
    setupConnect();
    setupIni();
    setupNetwork();
    setupFinish();

    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    QString filename;
    if(mIsAdmin){
        filename = QStringLiteral("admin.ini");
    }
    else{
        filename = QStringLiteral("player.ini");
    }
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << filename << " not found";
        qWarning("Couldn't open .ini file");
        return;
    }


    QJsonObject gameObject;
    if(mIsAdmin){
        gameObject["savegame"] = mCurrentFile;
    }
    else{
        gameObject["serverIP"] = mMasterIp;
    }

    QJsonDocument saveDoc(gameObject);
    saveFile.write(saveDoc.toJson());

    qDebug() << filename << " saved";
}


void MainWindow::setupActions()
{
    mUndoStack = new QUndoStack(this);


    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = fileMenu->addAction(tr("&Open..."), this, &MainWindow::chooseFileToLoad);
    openAction->setShortcuts(QKeySequence::Open);

    QAction *saveAction = fileMenu->addAction(tr("&Save..."), this, &MainWindow::saveFile);
    saveAction->setShortcuts(QKeySequence::Save);

    QAction *saveAsAction = fileMenu->addAction(tr("Save As..."), this, &MainWindow::saveAsFile);
    saveAsAction->setShortcuts(QKeySequence::SaveAs);

    QAction *exitAction = fileMenu->addAction(tr("E&xit"), qApp, &QCoreApplication::quit);
    exitAction->setShortcuts(QKeySequence::Quit);



    displayFamilyAction = new QAction(tr("&Display family"), this);
    displayFamilyAction->setCheckable(true);
    displayFamilyAction->setChecked(true);

    deleteAction = new QAction(tr("&Delete Item"), this);
    deleteAction->setShortcut(tr("Del"));
    //    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    undoAction = mUndoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);

    redoAction = mUndoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);


    mScreenshotAction = new QAction("Screenshot", this);

    mForceRefreshClientAction = new QAction("Force refresh");
    mPushOrdersClientAction = new QAction("Confirm orders");

    mPushRefreshAdminAction = new QAction("Push refresh");
    mPushUnitUpdateAdminAction = new QAction("Push unit update");
    mTestAdminAction = new QAction("Do a test");
    mPrintDebugAdminAction = new QAction("Print debug");
    mTimerCountdownAction = new QAction("Timer countdown");

    // QMenu *actionMenu = menuBar()->addMenu(tr("&Action"));
    // actionMenu->addAction(displayFamilyAction);
    // actionMenu->addAction(deleteAction);
    // actionMenu->addAction(undoAction);
    // actionMenu->addAction(redoAction);
    // actionMenu->addAction(mScreenshotAction);
    // actionMenu->hide(); // HACK 2024

    // if(mIsAdmin){
    //     QMenu *adminMenu = menuBar()->addMenu(tr("&Admin"));
    //     adminMenu->addAction(mPushRefreshAdminAction);
    //     adminMenu->addAction(mPushUnitUpdateAdminAction);
    //     adminMenu->addAction(mTestAdminAction);
    //     adminMenu->addAction(mPrintDebugAdminAction);
    //     adminMenu->addAction(mTimerCountdownAction);
    //     adminMenu->hide(); // HACK 2024
    // }


    // QMenu *playerMenu = menuBar()->addMenu(tr("&Player"));
    // playerMenu->addAction(mForceRefreshClientAction);
    // playerMenu->addAction(mPushOrdersClientAction);
    // playerMenu->hide(); // HACK 2024


    mDisplayAllAction = new QAction("Display All", this);
    mDisplayAllAction->setShortcut(QKeySequence("F1"));
    mDisplayMacroAction = new QAction("Display Less", this);
    mDisplayMacroAction->setShortcut(QKeySequence("F2"));
    mDisplayMicroAction = new QAction("Display More", this);
    mDisplayMicroAction->setShortcut(QKeySequence("F3"));

    mUnitAction = new QAction("Units", this);
    mUnitAction->setShortcut(QKeySequence("F4"));
    mUnitAction->setCheckable(true);

    mDrawingAction = new QAction("Control Measures", this);
    mDrawingAction->setShortcut(QKeySequence("F5"));
    mDrawingAction->setCheckable(true);

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(mUnitAction);
    actionGroup->addAction(mDrawingAction);
    mUnitAction->setChecked(true);


    mAdminViewAction = new QAction("Admin", this);
    mAdminViewAction->setCheckable(true);

    mViewActionGroup = new QActionGroup(this);


    QToolBar * aggregationToolBar = new QToolBar(QString("Aggregation"), this);
    addToolBar(aggregationToolBar);

    // HACK 2024
    //    aggregationToolBar->addAction(mDisplayAllAction);
    //    aggregationToolBar->addAction(mDisplayMacroAction);
    //    aggregationToolBar->addAction(mDisplayMicroAction);
    //    aggregationToolBar->addSeparator();
    //    aggregationToolBar->addAction(mExploreAction);
    //    aggregationToolBar->addAction(mEditAction);
    //    aggregationToolBar->addSeparator();

    aggregationToolBar->addAction(mDisplayAllAction);
    aggregationToolBar->addAction(mDisplayMacroAction);
    aggregationToolBar->addAction(mDisplayMicroAction);
    aggregationToolBar->hide(); // HACK 2024

    QToolBar * modeToolBar = new QToolBar(QString::fromLatin1("Mode"), this);
    addToolBar(modeToolBar);

    modeToolBar->addAction(mUnitAction);
    modeToolBar->addAction(mDrawingAction);
    modeToolBar->hide(); // HACK 2024

    mViewToolbar = new QToolBar(QString("View"), this); // HACK 2024
    mViewToolbar->hide(); // HACK 2024
    //addToolBar(mViewToolbar); // HACK 2024
    //setupToolbar(); // HACK 2024
}


void MainWindow::setupToolbar()
{
    mViewToolbar->clear();
    //mViewToolbar->setVisible(true); // HACK 2024
    // Admin can switch view with anyone
    if(mIsAdmin){
        // HACK 2024
        // mViewToolbar->addAction(mAdminViewAction);
        // for(QAction * playerAction : mPlayerViewActions){
        //     mViewToolbar->addAction(playerAction);
        // }
    }
    // Non admin can only see their own player
    else{
        if(mPlayerViewActions.contains(mSessionName)){
            mViewToolbar->addAction(mPlayerViewActions[mSessionName]);
        }
        else{
            mViewToolbar->hide();
        }
    }

}


void MainWindow::setupGUI()
{
    QWidget * centralWidget = new QWidget(this);


    // Tree View
    mTreeView = new QTreeView(centralWidget);
    mTreeView->setObjectName(QStringLiteral("view"));
    mTreeView->setAlternatingRowColors(true);
    mTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    mTreeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    mTreeView->setAnimated(true);
    mTreeView->setAllColumnsShowFocus(true);
    mTreeView->setItemDelegate(new TreeDelegate(mData, centralWidget));
    mTreeView->setAutoExpandDelay(500);
    mTreeView->setModel(mModel);
    mTreeView->setExpandsOnDoubleClick(false);
    mTreeView->header()->setStretchLastSection(false);
    mTreeView->header()->setSectionResizeMode(0, QHeaderView::Interactive);

    mTreeView->setDragEnabled(true);
    mTreeView->viewport()->setAcceptDrops(true);
    mTreeView->setAcceptDrops(true);
    mTreeView->setDropIndicatorShown(true);
    mTreeView->setDragDropMode(QAbstractItemView::InternalMove);


    // Central widget
    mCentralTabWidget = new QTabWidget(centralWidget);

    mTaskOrgGraphicsView = new QGraphicsView(mCentralTabWidget);
    mTaskOrgGraphicsView->setRenderHint(QPainter::Antialiasing);
    mTaskOrgGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    mTaskOrgGraphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    mTaskOrgGraphicsView->setMouseTracking(true);
    mTaskOrgGraphicsView->viewport()->setMouseTracking(true);
    mTaskOrgGraphicsView->setScene(mTaskOrgScene);

    mMapGraphicsView = new QGraphicsView(mCentralTabWidget);
    mMapGraphicsView->setRenderHint(QPainter::Antialiasing);
    mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    //    mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);
    mMapGraphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    mMapGraphicsView->setScene(mMapScene);
    mMapGraphicsView->setAcceptDrops(true);
    mMapGraphicsView->viewport()->setAcceptDrops(true);
    //    ZoomEventFilter * z = new ZoomEventFilter(mMapGraphicsView);
    new ZoomEventFilter(mMapGraphicsView); // Nice, no leak


    // Controls
    mTimeLabel = new QLabel("--:--:--", this);
    mTimeEdit = new QDateTimeEdit(this);
    mTimeEdit->setDisplayFormat("hh:mm:ss");
    mTimeline = new QSlider(Qt::Horizontal, mCentralTabWidget);
    mAdvanceButton = new QPushButton("Advance", mCentralTabWidget);
    mConfirmOrdersButton = new QPushButton("Confirm orders", mCentralTabWidget);
    QHBoxLayout * mapLayout1 = new QHBoxLayout(mCentralTabWidget);
    if(mIsAdmin){
        mapLayout1->addWidget(mTimeEdit);
        mTimeLabel->hide();
    }
    else{
        mapLayout1->addWidget(mTimeLabel);
        mTimeEdit->hide();
    }
    //    mapLayout1->addWidget(mTimeline);
    mTimeline->hide();
    mapLayout1->addWidget(mAdvanceButton);
    mapLayout1->addWidget(mConfirmOrdersButton);
    if(mIsObserver){
        mConfirmOrdersButton->hide();
    }
    mapLayout1->setStretch(0, 10);
    mapLayout1->setStretch(1, 1);
    QWidget * mapWidget1 = new QWidget(mCentralTabWidget);
    mapWidget1->setLayout(mapLayout1);

    if(mIsAdmin){
        mConfirmOrdersButton->hide();
    }
    else{
        mAdvanceButton->hide();
    }

    QVBoxLayout * mapLayout2 = new QVBoxLayout(mCentralTabWidget);
    mapLayout2->addWidget(mapWidget1);
    mapLayout2->addWidget(mMapGraphicsView);
    mapLayout2->setStretch(0, 1);
    mapLayout2->setStretch(1, 10);
    QWidget * mapWidget2 = new QWidget(mCentralTabWidget);
    mapWidget2->setLayout(mapLayout2);


    mCentralTabWidget->addTab(mTaskOrgGraphicsView, "ODB"); // HACK 2024
    mCentralTabWidget->addTab(mapWidget2, "Map");
    mCentralTabWidget->setCurrentIndex(0);

    // Side left
    QGroupBox * treeContainer = new QGroupBox("Hierarchy", centralWidget);
    QGridLayout * treeContainerLayout = new QGridLayout(treeContainer);
    treeContainer->setLayout(treeContainerLayout);
    treeContainerLayout->addWidget(mTreeView, 0, 0, 8, 3);
    QPushButton * moveUpButton = new QPushButton("Up", centralWidget);
    QPushButton * moveDownButton = new QPushButton("Down", centralWidget);
    treeContainerLayout->addWidget(moveUpButton, 8, 0, 1, 1);
    treeContainerLayout->addWidget(moveDownButton, 8, 1, 1, 1);
    moveUpButton->hide();
    moveDownButton->hide();

    // Side right
    controlContainer = new QGroupBox("Display control", centralWidget);
    QGroupBox * detailContainer = new QGroupBox("Unit details", centralWidget);

    mUnitDetail = new UnitDetail(mData, this);

    QGroupBox * timeContainer = new QGroupBox("Time remaining", this);
    mLCD = new QLCDNumber(this);
    mLCD->display("--:--");
    QLabel * spinBoxLabel = new QLabel("Countdown minutes", this);
    mSpinBox = new QSpinBox(this);
    mSpinBox->setMinimum(1);
    mSpinBox->setMaximum(20);
    mSpinBox->setValue(10);
    QGridLayout * timeContainerLayout = new QGridLayout(timeContainer);
    timeContainerLayout->addWidget(mLCD, 0, 0, 1, 2);
    timeContainerLayout->addWidget(spinBoxLabel, 1, 1);
    timeContainerLayout->addWidget(mSpinBox, 1, 1);
    if(!mIsAdmin){
        mSpinBox->hide();
    }
    timeContainer->setLayout(timeContainerLayout);

    QGridLayout * detailContainerLayout = new QGridLayout(detailContainer);
    detailContainer->setLayout(detailContainerLayout);
    detailContainerLayout->addWidget(mUnitDetail);

    QSplitter * rightContainer = new QSplitter(Qt::Vertical, centralWidget);
    rightContainer->addWidget(timeContainer);
    rightContainer->addWidget(controlContainer);
    rightContainer->addWidget(detailContainer);
    timeContainer->hide(); // HACK 2024
    controlContainer->hide(); // HACK 2024

    QLabel * textSizeLabel = new QLabel("Font size : ", centralWidget);
    textSizeSlider = new QSlider(Qt::Horizontal, this);
    textSizeSlider->setMinimum(100);
    textSizeSlider->setMaximum(1000);
    textSizeSlider->setValue(350);

    QGroupBox * groupBox = new QGroupBox("Draw type", this);
    buttonNoDraw = new QPushButton("Selection", this);
    buttonLine = new QPushButton("Line", this);
    buttonCircle = new QPushButton("Circle", this);
    buttonRectangle = new QPushButton("Rectangle", this);
    buttonLine->setCheckable(true);
    buttonCircle->setCheckable(true);
    buttonRectangle->setCheckable(true);
    buttonNoDraw->setCheckable(true);
    buttonNoDraw->setChecked(true);
    QButtonGroup * buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(buttonLine);
    buttonGroup->addButton(buttonCircle);
    buttonGroup->addButton(buttonRectangle);
    buttonGroup->addButton(buttonNoDraw);

    QVBoxLayout * groupBoxLayout = new QVBoxLayout;
    QHBoxLayout * drawGroupBoxLayout = new QHBoxLayout;
    drawGroupBoxLayout->addWidget(buttonLine);
    drawGroupBoxLayout->addWidget(buttonCircle);
    drawGroupBoxLayout->addWidget(buttonRectangle);
    groupBoxLayout->addWidget(buttonNoDraw);
    groupBoxLayout->addLayout(drawGroupBoxLayout);
    groupBox->setLayout(groupBoxLayout);

    mPolyLineSettings = new VEPolyLineSettings(mData, this);
    mShapeSettings = new VEShapeSettings(mData, this);

    //    mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    //        mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);


    // VE
    connect(buttonNoDraw,       &QPushButton::toggled, [this](bool checked){
        if(!checked) {return;}
        qDebug() << "map drag " << mMapGraphicsView->dragMode();
        mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        qDebug() << "map drag " << mMapGraphicsView->dragMode();
        mMapScene->setCurrentAction(MyMapGraphicsScene::DefaultType);});
    connect(buttonLine,         &QPushButton::clicked, [this](){
        //        qDebug() << "map drag " << mMapGraphicsView->dragMode();
        mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);
        //        qDebug() << "map drag " << mMapGraphicsView->dragMode();
        mMapScene->setCurrentAction(MyMapGraphicsScene::LineType);});
    connect(buttonRectangle,    &QPushButton::clicked, [this](){
        mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);
        mMapScene->setCurrentAction(MyMapGraphicsScene::RectangleType);});
    connect(buttonCircle,       &QPushButton::clicked, [this](){
        mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);
        mMapScene->setCurrentAction(MyMapGraphicsScene::EllipseType);});


    QGridLayout * controlContainerLayout = new QGridLayout(controlContainer);
    controlContainer->setLayout(controlContainerLayout);
    controlContainerLayout->addWidget(textSizeLabel, 0, 0);
    controlContainerLayout->addWidget(textSizeSlider, 0, 1);
    controlContainerLayout->addWidget(groupBox, 1, 0, 1, 2);
    controlContainerLayout->addWidget(mPolyLineSettings, 2, 0, 1, 2);
    controlContainerLayout->addWidget(mShapeSettings, 3, 0, 1, 2);


    QSplitter * splitter = new QSplitter(this);
    splitter->addWidget(treeContainer);
    splitter->addWidget(mCentralTabWidget);
    //splitter->addWidget(rightContainer); // HACK 2024
    splitter->setSizes(QList<int>{200,600});

    // HACK 2024
    QSplitter * splitter2024 = new QSplitter(this);
    splitter2024->setOrientation(Qt::Vertical);
    splitter2024->addWidget(rightContainer);
    splitter2024->addWidget(splitter);
    //splitter->addWidget(rightContainer); // HACK 2024
    splitter2024->setSizes(QList<int>{200,800});

    QGridLayout * layout = new QGridLayout(centralWidget);
    layout->addWidget(splitter2024, 0, 0, 4, 6);
    // HACK 2024
    //layout->addWidget(rightContainer, 0, 0, 1, 4);
    //layout->addWidget(splitter, 1, 0, 4, 4);


    centralWidget->setLayout(layout);

    setCentralWidget(centralWidget);

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupConnect()
{
    // DATA TO MAIN WINDOW
    connect(mData, &Data::playersLoaded,        this, &MainWindow::setupPlayers);

    connect(mAdminViewAction, &QAction::toggled, this, &MainWindow::setAdminView);
    connect(mData, &Data::clearUnitVisibility, mMapScene, &MyMapGraphicsScene::clearUnitVisibility);
    connect(mData, &Data::setUnitVisibility, mMapScene, &MyMapGraphicsScene::setUnitVisibility);
    connect(mData, &Data::setUnitVisibilityLife, mMapScene, &MyMapGraphicsScene::setUnitVisibilityLife);
    connect(mData, &Data::setAdminUnitVisibility, mMapScene, &MyMapGraphicsScene::setAdminUnitVisibility);

    connect(mData, &Data::drawingCreated, mMapScene, &MyMapGraphicsScene::createDrawing);
    connect(mData, &Data::drawingDeleted, mMapScene, &MyMapGraphicsScene::deleteDrawing);
    connect(mData, &Data::drawingUpdated, mMapScene, &MyMapGraphicsScene::updateDrawing);

    // DATA TO VIEW MODEL
    // There are lambdas because the view model can't interact with the view
    // so we have to explicitely tell treeView to update the rendering
    connect(mData, &Data::uniqueIdAdded,        mModel, &MyStandardItemModel::addUniqueId);
    connect(mData, &Data::uniqueIdDeleted,      mModel, &MyStandardItemModel::deleteUniqueId);
    connect(mData, &Data::unitContentUpdated,   [this](Unit unit){
        mModel->updateUnitContent(unit);
        mTreeView->viewport()->update();});
    connect(mData, &Data::unitParentRemoved, [this](int uniqueId){
        mModel->removeUnitParent(uniqueId);
        mTreeView->viewport()->update();});
    connect(mData, &Data::unitParentChanged, [this](int parentId, int childId){
        mModel->changeUnitParent(parentId, childId);
        mTreeView->viewport()->update();});
    connect(mData, &Data::unitSelected,[this](int uniqueId){
        if(!adminView()){
            QString playerName = playerView();
            if(!(mData->getTruePlayers()[playerName]->ownedUnits().contains(uniqueId))){
                return;
            }
        }
        QModelIndex index = mModel->indexFromId(uniqueId);
        //                qDebug() << "Select in the treeView the unit with ID : " << uniqueId << " and with index : " << index;
        mTreeView->scrollTo(index);
        //        treeView->expand(index);
        mTreeView->resizeColumnToContents(1);
        mTreeView->viewport()->update();
    });
    connect(mData, &Data::dataCleared,          mModel, &MyStandardItemModel::clear);
    connect(mData, &Data::unitsCleared,          mModel, &MyStandardItemModel::clear);
    connect(mData, &Data::debugSignal,          mModel, &MyStandardItemModel::debug);



    // DATA TO Task org
    connect(mData, &Data::uniqueIdAdded,        mTaskOrgScene, &MyTaskOrgGraphicsScene::addUniqueId);
    connect(mData, &Data::uniqueIdDeleted,      mTaskOrgScene, &MyTaskOrgGraphicsScene::deleteUniqueId);
    connect(mData, &Data::unitContentUpdated,   mTaskOrgScene, &MyTaskOrgGraphicsScene::updateUnitContent);
    connect(mData, &Data::unitParentRemoved,    mTaskOrgScene, &MyTaskOrgGraphicsScene::removeUnitParent);
    connect(mData, &Data::unitParentChanged,    mTaskOrgScene, &MyTaskOrgGraphicsScene::changeUnitParent);
    connect(mData, &Data::unitSelected,         mTaskOrgScene, &MyTaskOrgGraphicsScene::selectUnit);
    connect(mData, &Data::dataCleared,          mTaskOrgScene, &MyTaskOrgGraphicsScene::clear);
    connect(mData, &Data::unitsCleared,         mTaskOrgScene, &MyTaskOrgGraphicsScene::clear);
    connect(mData, &Data::debugSignal,          mTaskOrgScene, &MyTaskOrgGraphicsScene::debug);

    // DATA TO Map
    connect(mData, &Data::mapLoaded,            mMapScene, &MyMapGraphicsScene::loadMap);
    connect(mData, &Data::uniqueIdAdded,        mMapScene, &MyMapGraphicsScene::addUniqueId);
    connect(mData, &Data::uniqueIdDeleted,      mMapScene, &MyMapGraphicsScene::deleteUniqueId);
    connect(mData, &Data::unitContentUpdated,   mMapScene, &MyMapGraphicsScene::updateUnitContent);
    connect(mData, &Data::unitPositionUpdated,  mMapScene, &MyMapGraphicsScene::updateUnitPosition);
    connect(mData, &Data::unitParentRemoved,    mMapScene, &MyMapGraphicsScene::removeUnitParent);
    connect(mData, &Data::unitParentChanged,    mMapScene, &MyMapGraphicsScene::changeUnitParent);
    connect(mData, &Data::unitSelected,         mMapScene, &MyMapGraphicsScene::selectUnit);
    connect(mData, &Data::dataCleared,          mMapScene, &MyMapGraphicsScene::clear);
    //    connect(mData, &Data::mapCleared,           mMapScene, &MyMapGraphicsScene::clear); // Hmmm
    connect(mData, &Data::unitsCleared,         mMapScene, &MyMapGraphicsScene::clearUnits);
    connect(mData, &Data::ordersCleared,        mMapScene, &MyMapGraphicsScene::clearOrders);
    connect(mData, &Data::debugSignal,          mMapScene, &MyMapGraphicsScene::debug);
    connect(mData, &Data::unitSelected,         this, &MainWindow::showUnitOnMap);


    // DATA TO UNIT Detail
    connect(mData, &Data::unitContentUpdated,   mUnitDetail, &UnitDetail::updateUnit);
    connect(mData, &Data::unitSelected,         mUnitDetail, &UnitDetail::selectUnit);




    // TREE VIEW
    connect(mTreeView, &QTreeView::clicked,          this, &MainWindow::unitClicked);
    connect(mTreeView, &QTreeView::doubleClicked,    this, &MainWindow::unitDoubleClicked);
    connect(mTreeView, &QTreeView::expanded,         [this](const QModelIndex &){mTreeView->resizeColumnToContents(0);});
    connect(mTreeView, &QTreeView::collapsed,        [this](const QModelIndex &){mTreeView->resizeColumnToContents(0);});
    connect(mTreeView->header(), &QHeaderView::sectionDoubleClicked, [=](int index){mTreeView->resizeColumnToContents(index);});

    // UNIT DETAIL
    connect(mUnitDetail, &UnitDetail::unitChanged, [this](Unit unit){
        UpdateUnitCommand * command = new UpdateUnitCommand(mData, unit);
        mUndoStack->push(command);});

    // TASK ORG
    connect(mTaskOrgScene, &MyTaskOrgGraphicsScene::unitSelected, mData, &Data::selectUnit);

    // MAP
    //    connect(mMapScene, &MyMapGraphicsScene::unitMoved, mData, &Data::updateUnitPosition);
    connect(mMapScene, &MyMapGraphicsScene::unitMoved, [this](int uniqueId, QPointF position){
        MoveUnitCommand * command = new MoveUnitCommand(mData, uniqueId, position);
        mUndoStack->push(command);
    });
    connect(mMapScene, &MyMapGraphicsScene::unitDropped,  [this](int uniqueId, QPointF pos){
        mData->updateUnitPosition(uniqueId, pos);
        mData->selectUnit(uniqueId);} );
    connect(mMapScene, &MyMapGraphicsScene::unitSelected, mData, &Data::selectUnit);




    // CONTROLS
    connect(displayFamilyAction, &QAction::toggled, mTaskOrgScene, &MyTaskOrgGraphicsScene::setDisplayFullFamily);
    connect(textSizeSlider, &QSlider::valueChanged, [=](int value){mTaskOrgScene->updateTextSize(value/100.0);});

    connect(mDisplayAllAction, &QAction::triggered, mMapScene, &MyMapGraphicsScene::displayAll);
    connect(mDisplayMacroAction, &QAction::triggered, mMapScene, &MyMapGraphicsScene::displayMacro);
    connect(mDisplayMicroAction, &QAction::triggered, mMapScene, &MyMapGraphicsScene::displayMicro);
    connect(mUnitAction, &QAction::toggled, this, &MainWindow::setUnitMode);
    connect(mDrawingAction, &QAction::toggled, this, &MainWindow::setDrawingMode);

    connect(mScreenshotAction, &QAction::triggered, this, &MainWindow::screenshot);


    connect(mMapScene, &MyMapGraphicsScene::drawingSelected, this, &MainWindow::drawingSelected);
    connect(mMapScene, &MyMapGraphicsScene::drawingCreatedByUser, this, &MainWindow::drawingCreatedByUser);
    connect(mMapScene, &MyMapGraphicsScene::drawingDeletedByUser, this, &MainWindow::drawingDeletedByUser);
    connect(mMapScene, &MyMapGraphicsScene::drawingUpdatedByUser, this, &MainWindow::drawingUpdatedByUser);
    connect(mMapScene, &MyMapGraphicsScene::drawingFinishedByUser, [this](){buttonNoDraw->setChecked(true);});



    connect(mTestAdminAction, &QAction::triggered, mData, &Data::debug);        // Do something fun
    connect(mPrintDebugAdminAction, &QAction::triggered, mData, &Data::debug);
    connect(mTimerCountdownAction, &QAction::triggered, this, &MainWindow::launchCountdown);

    connect(mPolyLineSettings, &VEPolyLineSettings::lineUpdated, this, &MainWindow::drawingUpdatedByUser);


    connect(mData, &Data::orderUpdated, this, &MainWindow::orderUpdatedByData);
    connect(mData, &Data::hideAllOrders, mMapScene, &MyMapGraphicsScene::hideAllOrders);
    connect(mData, &Data::showOrders, mMapScene, &MyMapGraphicsScene::showOrders);

    connect(mAdvanceButton, &QPushButton::clicked, this, &MainWindow::advanceState);
    connect(mData, &Data::timeUpdated, this, &MainWindow::updateTime);

    connect(mConfirmOrdersButton, &QPushButton::clicked, [this](){
        QPalette p(mConfirmOrdersButton->palette());
        p.setColor(QPalette::Button, QColor("#00ff00"));
        mConfirmOrdersButton->setAutoFillBackground(true);
        mConfirmOrdersButton->setPalette(p);
        mConfirmOrdersButton->repaint();});
    connect(mData, &Data::orderUpdated, [this](Order order){
        Q_UNUSED(order);
        QPalette p(mConfirmOrdersButton->palette());
        p.setColor(QPalette::Button, QColor("#ff0000"));
        mConfirmOrdersButton->setAutoFillBackground(true);
        mConfirmOrdersButton->setPalette(p);
        mConfirmOrdersButton->repaint();});


    connect(mTimeEdit, &QDateTimeEdit::dateTimeChanged, mData, &Data::changeTime);
    connect(mTimer, &QTimer::timeout, this, &MainWindow::updateCountdown);

}


// Read stuff from ini file
void MainWindow::setupIni()
{
    // Load ini file
    mCurrentPath = QDir::currentPath();
    //    qDebug() << "Current path : " << currentPath;

    QString filename;
    if(mIsAdmin){
        filename = QStringLiteral("admin.ini");
    }
    else{
        filename = QStringLiteral("player.ini");
    }

    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open .ini file";
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData));
    const QJsonObject & jsonObject = jsonDoc.object();

    // admin stuff
    // - load the last game opened
    if(mIsAdmin){
        if (jsonObject.contains("savegame") && jsonObject["savegame"].isString()) {

            // Open the file
            QString saveFilename =jsonObject["savegame"].toString();
            qDebug() <<"Found save " << saveFilename;
            if(!openFile(saveFilename)){
                return;
            }

            // Do some ease of use
            if(mData->units().contains(0))
            {
                mTreeView->expand(mModel->indexFromId(0));
                emit mData->selectUnit(0);
            }
            mTreeView->resizeColumnToContents(1);
            mTreeView->resizeColumnToContents(2);
            mTreeView->resizeColumnToContents(3);
            mUndoStack->clear(); // To prevent someone from Ctrl-Z and unload the game

            mData->computeVisibility();
        }
        else{
            qDebug() <<"No savegame found";
        }
    }
    // player stuff
    else{
        if (jsonObject.contains("serverIP") && jsonObject["serverIP"].isString()){
            mMasterIp = jsonObject["serverIP"].toString();
        }
        else{
            mMasterIp  = QStringLiteral("localhost");
        }

        if(mSessionName.isEmpty()){
            if (jsonObject.contains("name") && jsonObject["name"].isString()){
                mSessionName = jsonObject["name"].toString();
            }
            else{
                QRandomGenerator * rng = QRandomGenerator::system();
                quint64 id = rng->generate64();
                mSessionName  = QStringLiteral("NoName_") + QString::number(id);
            }
        }


    }
}

void MainWindow::setupNetwork()
{
    if(mIsAdmin){
        mServer = new NetworkServer(mData, 1234, this);
        connect(mServer, &NetworkServer::infoUpdated, this, &MainWindow::outputInfoMessage);
        connect(mPushRefreshAdminAction, &QAction::triggered, mServer, &NetworkServer::pushCurrentState);
        connect(mPushUnitUpdateAdminAction, &QAction::triggered, mServer, &NetworkServer::pushUnitUpdate);
        connect(mPushUnitUpdateAdminAction, &QAction::triggered, this, &MainWindow::autosave);
    }
    else{
        mClient = new NetworkClient(mData, QUrl(QStringLiteral("ws://") + mMasterIp + QStringLiteral(":1234")), this);

        connect(mClient, &NetworkClient::infoUpdated, this, &MainWindow::outputInfoMessage);
        connect(mForceRefreshClientAction, &QAction::triggered, mClient, &NetworkClient::requestCurrentState);
        connect(mPushOrdersClientAction, &QAction::triggered, mClient, &NetworkClient::pushOrders);
        connect(mConfirmOrdersButton, &QPushButton::clicked, mClient, &NetworkClient::pushOrders);
    }
}

void MainWindow::setupFinish()
{
    if(!mIsAdmin){
        mClient->connectToServer();
        if(!(mData->getPlayerNames().empty())){
            setPlayerView(mData->getPlayerNames()[0]);
        }
        mUnitDetail->setDisabled(true);
        mTreeView->setDisabled(true);
    }
    else{
        setAdminView();
    }
    qDebug() << "map drag " << mMapGraphicsView->dragMode();
    setUnitMode(true);

    //    emit mData->debug();
}

void MainWindow::setupPlayers(const QMap<QString, Player *> & players)
{
    // Clear the action group
    for(QAction * action : mViewActionGroup->actions()){
        mViewActionGroup->removeAction(action);
    }

    // Clear the action themselves
    // TODO delete the actions
    for(QAction * action : mPlayerViewActions){
        action->deleteLater();
    }
    mPlayerViewActions.clear();



    // Create all the actions
    for(auto it = players.constBegin(); it != players.constEnd(); ++it){
        QString playerName = it.key();
        Player * player = it.value();
        QAction * playerAction = new QAction(playerName, this);
        playerAction->setCheckable(true);
        mPlayerViewActions[playerName] = playerAction;
        connect(playerAction, &QAction::toggled, [this, playerName](){setPlayerView(playerName);});
    }

    // Put all the actions in a QActionGroup
    mViewActionGroup->addAction(mAdminViewAction);
    for(QAction * action : mPlayerViewActions){
        mViewActionGroup->addAction(action);
    }

    // Check the right view
    if(mIsAdmin){
        mAdminViewAction->setChecked(true);
    }
    else{
        if(mPlayerViewActions.contains(mSessionName)){
            mPlayerViewActions[mSessionName]->setChecked(true);
        }
        else{
            qWarning() << "Name not in the player list";
        }
    }

    // Toolbar
    setupToolbar();
}

///////////////////////////////////////////////////////////////////////
/// GETTERS/SETTERS                                                 ///
///////////////////////////////////////////////////////////////////////

QString MainWindow::sessionName() const
{
    return mSessionName;
}

void MainWindow::setSessionName(const QString &sessionName)
{
    mSessionName = sessionName;

}


///////////////////////////////////////////////////////////////////////
/// SLOTS/ACTIONS                                                   ///
///////////////////////////////////////////////////////////////////////

void MainWindow::chooseFileToLoad()
{
    //    qDebug() << "Choose a file to open";

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose an image"), mCurrentPath, "*");

    if (!fileName.isEmpty()){
        openFile(fileName);
    }
}

bool MainWindow::openFile(const QString &filename)
{
    //    qDebug() << "Open file " << filename;
    mCurrentFile = filename;

    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Trying to open non existent file " << filename;
        return false;
    }

    LoadCommand * command = new LoadCommand{mData, filename};
    mUndoStack->push(command);
    return true;
}

void MainWindow::saveFile()
{
    qDebug() << "Save file";
    mData->saveGame(mCurrentFile);
}

void MainWindow::saveAsFile()
{

    qDebug() << "Save as file";

    QString filename = QFileDialog::getSaveFileName(
                this, tr("Choose a savefile"), mCurrentPath, "*.json");

    if (!filename.isEmpty()){
        mCurrentFile = filename;
        mData->saveGame(filename);
    }
}

void MainWindow::screenshot()
{
    if(mCentralTabWidget->currentIndex() == 0){
        QRectF sceneRect = mTaskOrgScene->sceneRect();
        QImage img(QSize(2*sceneRect.width(), 2*sceneRect.height()), QImage::Format_ARGB32);
        //    QImage img(QSize(sceneRect), QImage::Format_ARGB32);
        QPainter painter(&img);
        mTaskOrgScene->render(&painter, img.rect(), sceneRect);
        QString filename = QDir::currentPath();
        filename += "/Screenshot_";
        filename += QString::number(QDateTime::currentMSecsSinceEpoch());
        filename += ".png";
        qDebug() << "Screenshot saved at " << filename;
        img.save(filename);
    }
    else
    {
        QRectF sceneRect = mMapScene->sceneRect();
        QImage img(QSize(2*sceneRect.width(), 2*sceneRect.height()), QImage::Format_ARGB32);
        //    QImage img(QSize(sceneRect), QImage::Format_ARGB32);
        QPainter painter(&img);
        mMapScene->render(&painter, img.rect(), sceneRect);
        QString filename = QDir::currentPath();
        filename += "/Screenshot_";
        filename += QString::number(QDateTime::currentMSecsSinceEpoch());
        filename += ".png";
        qDebug() << "Screenshot saved at " << filename;
        img.save(filename);
    }
}

void MainWindow::unitClicked(const QModelIndex &index)
{
    int uniqueId = index.sibling(index.row(), 0).data().toInt();
    qDebug() << "Click on unit " << uniqueId << " at index " << index;

    switch(index.column()){
    case 0:
        mData->selectUnit(uniqueId);
        break;
    default:
        break;
    }
    // Some ugly hack because the usual and direct resize does not work
    // treeView->resizeColumnToContents(0);
    //    QTimer::singleShot(1, [this](){treeView->resizeColumnToContents(0);});
}



void MainWindow::unitDoubleClicked(const QModelIndex &index)
{
    int uniqueId = index.sibling(index.row(), 0).data().toInt();
    qDebug() << "Double click on unit " << uniqueId;
    switch(index.column()){
    case 2:{

        AddUnitCommand * command = new AddUnitCommand(mData, mData->getTheoreticalChild(uniqueId));
        mUndoStack->push(command);
        break;
    }

    case 3:{
        auto units = mData->units();
        Unit unit = Unit(*(units[uniqueId]));
        DeleteUnitCommand * command = new DeleteUnitCommand(mData, uniqueId);
        mUndoStack->push(command);
        break;
    }

    default:
        break;
    }
}

void MainWindow::showUnitOnMap(int uniqueId)
{
    QGraphicsItem * item = mMapScene->getUnit(uniqueId);
    if(!item){
        return;
    }
    if(!item->isVisible()){
        MapUnitItem * mapUnitItem = qgraphicsitem_cast<MapUnitItem*>(item);
        mMapScene->setUnitVisible(mapUnitItem, true);
        //        item->setVisible(true);
        //        return;
    }
    if(!item->isVisible()){
        return;
    }

    QRect viewport_rect(0, 0, mMapGraphicsView->viewport()->width(), mMapGraphicsView->viewport()->height());
    QRectF visible_scene_rect = mMapGraphicsView->mapToScene(viewport_rect).boundingRect();
    if(visible_scene_rect.contains(item->scenePos())){
        return;
    }

    QPointF oldCenter = visible_scene_rect.center();
    QPointF newCenter = item->scenePos();

    int nbMilliseconds = 200;
    int updateTimeMs = 20; // update every 20 ms

    int * iteration = new int(0);
    int * maxIteration = new int(nbMilliseconds/updateTimeMs);
    QTimer* pTimer = new QTimer;
    QObject::connect(pTimer, &QTimer::timeout, [=](){
        double factor = (1.0*(*iteration)/(1.0*(*maxIteration)));

        mMapGraphicsView->centerOn(newCenter*factor + oldCenter*(1.0-factor));

        (*iteration)++;
        // check for end position or time, then....
        if(*iteration > *maxIteration)
        {
            pTimer->stop();
            pTimer->deleteLater();
            delete iteration;
            delete maxIteration;
        }
    });
    pTimer->start(updateTimeMs);
}

void MainWindow::outputInfoMessage(const QString &message)
{
    qDebug() << message;
}

void MainWindow::updateTime(const QString &time)
{
    if(mIsAdmin){
        mTimeEdit->setDateTime(QDateTime::fromString(time, "hh:mm:ss"));
    }
    else{
        mTimeLabel->setText(time);
    }
}

bool MainWindow::getAdminView() const
{
    return mAdminView;
}

void MainWindow::autosave()
{
    QDateTime dt = QDateTime::currentDateTime();
    mData->saveGame(dt.toString("MMM-dd--hh-mm-ss") + ".json");
}

bool MainWindow::isObserver() const
{
    return mIsObserver;
}

QGraphicsView *MainWindow::getMapGraphicsView() const
{
    return mMapGraphicsView;
}

void MainWindow::drawingSelected(Drawing drawing)
{
    switch (drawing.type()) {
    case Drawing::ELLIPSE:
    case Drawing::RECTANGLE: {
        qDebug() << "Shape selected";
        //        mShapeSettings->loadShape(drawing);
        mShapeSettings->setVisible(true);
        mPolyLineSettings->setVisible(false);
        break;
    }


    case Drawing::POLYLINE: {
        qDebug() << "Polyline selected";
        mPolyLineSettings->loadPolyline(drawing);
        mPolyLineSettings->setVisible(true);
        mShapeSettings->setVisible(false);
        break;
    }
    default:
        break;
    }
}

void MainWindow::drawingCreatedByUser(Drawing drawing)
{


    switch (drawing.type()) {
    case Drawing::ELLIPSE :
    case Drawing::RECTANGLE: {
        qDebug() << "New shape created by user";
        //        mShapeSettings->newShape(drawing);
        mShapeSettings->setVisible(true);
        mPolyLineSettings->setVisible(false);
        break;
    }
    case Drawing::POLYLINE: {
        qDebug() << "New polyline created by user";
        drawing = mPolyLineSettings->newPolyline(drawing);
        mPolyLineSettings->setVisible(true);
        mShapeSettings->setVisible(false);
        break;
    }
    default:
        break;
    }

    AddDrawingCommand * command = new AddDrawingCommand(mData, drawing);
    mUndoStack->push(command);

}

void MainWindow::drawingDeletedByUser(int uniqueId)
{
    DeleteDrawingCommand * command = new DeleteDrawingCommand(mData, uniqueId);
    mUndoStack->push(command);
}

void MainWindow::drawingUpdatedByUser(Drawing drawing)
{
    UpdateDrawingCommand * command = new UpdateDrawingCommand(mData, *(mData->drawings()[drawing.uniqueId()]), drawing);
    mUndoStack->push(command);
}

void MainWindow::orderUpdatedByUser(Order order)
{
    // Admin view isn't supposed to be able to edit orders
    if(adminView()){
        qDebug() << "Order update rejected because admin view";
        return;
    }

    // If the player doesn't exist, do nothing
    QString player = playerView();
    QMap<QString, QSet<int> > allUnits = mData->getPlayersOwnedUnits();
    if(!(allUnits.contains(player))){

        qDebug() << "Order update rejected because unkown player " << player;
        return;
    }

    // If the player doesn't own the origin of the order, do nothing
    QSet<int> ownedUnits = allUnits[player];
    if(!(ownedUnits.contains(order.originUniqueId()))){

        qDebug() << "Order update rejected because player " << player << " doesn't own unit " << order.originUniqueId();
        return;
    }

    mData->updateOrderFromUser(order);
    //    emit orderUpdated(order);
}

void MainWindow::orderUpdatedByData(Order order)
{
    //    // Admin view isn't supposed to be able to edit orders
    //    if(adminView()){
    //        qDebug() << "Updating order form data skipped because it is admin view";
    //        return;
    //    }

    //    // If the player doesn't exist, do nothing
    //    QString player = playerView();
    //    QMap<QString, QSet<int> > allUnits = mData->getPlayerOwnedUnits();
    //    if(!(allUnits.contains(player))){
    //        qDebug() << "Updating order form data skipped because player is unknown " << player;
    //        return;
    //    }

    //    // If the player doesn't own the origin of the order, do nothing
    //    QSet<int> ownedUnits = allUnits[player];
    //    if(!(ownedUnits.contains(order.originUniqueId()))){
    //        qDebug() << "Updating order form data skipped because player " << player << " doesn't own unit " << order.originUniqueId();
    //        return;
    //    }

    mMapScene->updateOrder(order);
}

void MainWindow::setAdminView()
{
    qDebug() << "Set admin view";
    mAdminView = true;
    mPlayerView.clear();

    mData->setAdminView();

}

void MainWindow::setPlayerView(const QString &player)
{
    qDebug() << "Set player view : " << player;
    mAdminView = false;
    mPlayerView = player;

    mData->setPlayerView(player);
}

void MainWindow::setUnitMode(bool b)
{
    if(!b){
        return;
    }
    qDebug() << "Units set to " << b;

    //    mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    buttonNoDraw->setDisabled(true);
    buttonLine->setDisabled(true);
    buttonCircle->setDisabled(true);
    buttonRectangle->setDisabled(true);

    mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);


    controlContainer->setVisible(false);
    mMapScene->setUnitMode();

    //    mMapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}

void MainWindow::setDrawingMode(bool b)
{
    if(!b){
        return;
    }
    qDebug() << "Control measures set to " << b;

    //    mMapGraphicsView->setDragMode(QGraphicsView::NoDrag);

    buttonNoDraw->setDisabled(false);
    buttonLine->setDisabled(false);
    buttonCircle->setDisabled(false);
    buttonRectangle->setDisabled(false);

    // Click click
    buttonNoDraw->toggle();
    if(!buttonNoDraw->isChecked()){
        buttonNoDraw->toggle();
    }

    controlContainer->setVisible(true);
    mMapScene->setDrawingMode();
}

void MainWindow::advanceState()
{
    mData->advanceState();
    // Time
    mMapScene->displayAll();
}

void MainWindow::launchCountdown()
{
    int nbSeconds = 60 * mSpinBox->value();
    mServer->pushTimeCountdown(nbSeconds);
    startCountdown(nbSeconds);
}

void MainWindow::startCountdown(int nbSeconds)
{
    mTimerNbSecondsRemaining = nbSeconds;
    mTimer->start(1000);

    QPalette palette = mLCD->palette();

    palette.setColor(palette.WindowText, QColor(0, 0, 0));
//    palette.setColor(palette.Background, QColor(0, 170, 255));
    palette.setColor(palette.Light, QColor(128, 128, 128));
    palette.setColor(palette.Dark, QColor(32, 32, 32));

    mLCD->setPalette(palette);
//    mMediaPlayer->setMedia(QUrl::fromLocalFile("qrc:/files/sound/car-ignition.wav"));
    mMediaPlayer->setMedia(QUrl("qrc:/files/sound/gare.wav"));
    mMediaPlayer->setVolume(50);
    mMediaPlayer->play();
}

void MainWindow::resetCountdown()
{
    mTimer->stop();
    mLCD->display("--:--");


    QPalette palette = mLCD->palette();

    palette.setColor(palette.WindowText, QColor(0, 0, 0));
//    palette.setColor(palette.Background, QColor(0, 170, 255));
    palette.setColor(palette.Light, QColor(128, 128, 128));
    palette.setColor(palette.Dark, QColor(32, 32, 32));

    mLCD->setPalette(palette);
}

void MainWindow::updateCountdown()
{
    mTimerNbSecondsRemaining--;
    qDebug() << "Time remaining " << mTimerNbSecondsRemaining;

    int nbMinutes = mTimerNbSecondsRemaining/60;
    int nbSeconds = mTimerNbSecondsRemaining - 60*nbMinutes;
    QTime time(0,0,0);
    time = time.addSecs(nbSeconds);
    time = time.addSecs(60*nbMinutes);
    QString display = time.toString("mm:ss");
    if(mTimerNbSecondsRemaining % 2 == 1){
        display[2] = QChar(' ');
    }
    qDebug() << display;
    mLCD->display(display);

    if(mTimerNbSecondsRemaining == 10){
        mMediaPlayer->setMedia(QUrl("qrc:/files/sound/kitchen-timer.wav"));
        mMediaPlayer->setVolume(50);
        mMediaPlayer->play();
    }
    if(mTimerNbSecondsRemaining == 0){
        mTimer->stop();

        QPalette palette = mLCD->palette();

        palette.setColor(palette.WindowText, QColor(192, 0, 0));
    //    palette.setColor(palette.Background, QColor(0, 170, 255));
        palette.setColor(palette.Light, QColor(255, 0, 0));
        palette.setColor(palette.Dark, QColor(128, 0, 0));


        mLCD->setPalette(palette);

    }
}

void MainWindow::endCountdown()
{

}

//QJsonObject MainWindow::getControlMeasures() const
//{
//    return mMapScene->getControlMeasures();
//}

//void MainWindow::setControlMeasures(QJsonObject &json)
//{
//    return mMapScene->setControlMeasures(json);
//}

//QJsonObject MainWindow::getIntentions() const
//{
//    return mMapScene->getIntentions();
//}

QString MainWindow::playerView() const
{
    return mPlayerView;
}

bool MainWindow::adminView() const
{
    return mAdminView;
}

bool MainWindow::isAdmin() const
{
    return mIsAdmin;
}

QUndoStack *MainWindow::undoStack() const
{
    return mUndoStack;
}


