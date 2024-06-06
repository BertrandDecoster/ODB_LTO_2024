#ifndef WIDGET_H
#define WIDGET_H


#include <QGraphicsItem>
#include <QJsonObject>
#include <QMainWindow>
#include <QPushButton>
#include <QStateMachine>

#include "drawing.h"
#include "order.h"

class QActionGroup;
class Data;
class MyStandardItemModel;
class MyTaskOrgGraphicsScene;
class MyMapGraphicsScene;
class UnitDetail;
class NetworkClient;
class NetworkServer;
class VEPolyLineSettings;
class VEShapeSettings;
class Player;
QT_BEGIN_NAMESPACE
class QMediaPlayer;
class QLCDNumber;
class QDateTimeEdit;
class QLabel;
class QGroupBox;
class QUndoStack;
class QTreeView;
class QGraphicsView;
class QComboBox;
class QAction;
class QSlider;
class QTabWidget;
class QTimer;
class QSpinBox;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    explicit MainWindow(bool isAdmin, QString sessionName, bool isObserver, QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private:
    void setupActions();
    void setupToolbar();
    void setupGUI();
    void setupConnect();
    void setupIni();
    void setupNetwork();
    void setupFinish();

private slots:
    void setupPlayers(const QMap<QString, Player *> &players);

    ///////////////////////////////////////////////////////////////////////
    /// GETTERS/SETTERS                                                 ///
    ///////////////////////////////////////////////////////////////////////
public:
    QString sessionName() const;
    void setSessionName(const QString &sessionName);

    QUndoStack *undoStack() const;
    bool isAdmin() const;


    ///////////////////////////////////////////////////////////////////////
    /// DATA MANIPULATION                                               ///
    ///////////////////////////////////////////////////////////////////////




public slots:
    void chooseFileToLoad();
    bool openFile(const QString & filename);
    void saveFile();
    void saveAsFile();
    void screenshot();

    // Lambdas would be too long
    // Tree view lambda
    void unitClicked(const QModelIndex & index);
    void unitDoubleClicked(const QModelIndex & index);
    // Map lambda
    void showUnitOnMap(int uniqueId);

    // When a control measure is picked, display the correct control box
    void drawingSelected(Drawing drawing);
    void drawingCreatedByUser(Drawing drawing);
    void drawingDeletedByUser(int uniqueId);
    void drawingUpdatedByUser(Drawing drawing);

    void orderUpdatedByUser(Order order);
    void orderUpdatedByData(Order order);


    ///////////////////////////////////////////////////////////////////////
    /// VIEW                                                            ///
    ///////////////////////////////////////////////////////////////////////
public :
    bool adminView() const;
    QString playerView() const;

    void setAdminView();
    void setPlayerView(const QString & player);


    QGraphicsView *getMapGraphicsView() const;

    bool isObserver() const;

public slots:
    void setUnitMode(bool b);
    void setDrawingMode(bool b);


    ///////////////////////////////////////////////////////////////////////
    /// NETWORK                                                         ///
    ///////////////////////////////////////////////////////////////////////
    //    QJsonObject getControlMeasures() const;
    //    void setControlMeasures(QJsonObject & json);
    //    QJsonObject getIntentions() const;
public slots:
    void advanceState();

    void launchCountdown();
    void startCountdown(int nbSeconds);
    void resetCountdown();
private slots:
    void updateCountdown();
    void endCountdown();

    ///////////////////////////////////////////////////////////////////////
    /// MISC                                                            ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void outputInfoMessage(const QString & message);
    void updateTime(const QString & time);

    bool getAdminView() const;

    void autosave();

private:
    // All the data, how to store it and how to sync with the other processes
    QUndoStack * mUndoStack;

    Data * mData;
    MyTaskOrgGraphicsScene * mTaskOrgScene;
    MyMapGraphicsScene * mMapScene;
    MyStandardItemModel * mModel;

    NetworkServer * mServer;
    NetworkClient * mClient;


    // GUI main

    QTabWidget * mCentralTabWidget;
    QGraphicsView * mTaskOrgGraphicsView;
    QGraphicsView * mMapGraphicsView;

    // GUI control
    // Left
    QTreeView * mTreeView;

    // Center
    QLabel * mTimeLabel;
    QDateTimeEdit * mTimeEdit;
    QSlider * mTimeline;
    QPushButton * mAdvanceButton;
    QPushButton * mConfirmOrdersButton;

    //Right Top
    QLCDNumber * mLCD;

    QComboBox * comboApp6;
    QSlider * textSizeSlider;
    VEPolyLineSettings * mPolyLineSettings;
    VEShapeSettings * mShapeSettings;

    QGroupBox * controlContainer;

    QPushButton * buttonNoDraw ;
    QPushButton * buttonLine ;
    QPushButton * buttonCircle ;
    QPushButton * buttonRectangle ;

    // Right Bottom
    UnitDetail * mUnitDetail;

    // QAction
    QAction * displayFamilyAction;
    QAction * deleteAction;
    QAction * undoAction;
    QAction * redoAction;
    QAction * mScreenshotAction;

    QAction * mPushRefreshAdminAction;
    QAction * mPushUnitUpdateAdminAction;
    QAction * mTestAdminAction;
    QAction * mPrintDebugAdminAction;
    QAction * mTimerCountdownAction;

    QAction * mForceRefreshClientAction;
    QAction * mPushOrdersClientAction;

    // F1-F5
    QAction * mDisplayAllAction;
    QAction * mDisplayMacroAction;
    QAction * mDisplayMicroAction;
    QAction * mUnitAction;
    QAction * mDrawingAction;

    QAction * mAdminViewAction;
    QMap<QString, QAction *> mPlayerViewActions;
    QActionGroup * mViewActionGroup;
    QToolBar * mViewToolbar;



    // Misc
    QStateMachine * mStateMachine; // Unused

    const bool mIsAdmin;
    const bool mIsObserver;
    QString mMasterIp;          // The one true god
    QString mSessionName;       // The name of this process
    //    QString mRoleName;          // The player name
    // TODO : currently, the name of the session IS the name of the player (if it exists)

    bool mAdminView;
    QString mPlayerView;

    QString mCurrentPath;
    QString mCurrentFile;

    // timer
    int mTimerNbSecondsRemaining;
    QTimer * mTimer;

    QMediaPlayer * mMediaPlayer;
    QSpinBox * mSpinBox;
};

#endif // WIDGET_H
