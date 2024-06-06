#ifndef UNITDETAIL_H
#define UNITDETAIL_H

#include <QWidget>

#include "unit.h"

class Unit;
class Data;
QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QLabel;
class QSlider;
QT_END_NAMESPACE

class UnitDetail : public QWidget
{
    Q_OBJECT


    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    UnitDetail(Data * data, QWidget * parent = nullptr);

protected:
    void setupGUI();
    void setupConnect();
    void setupApp6ComboBox();
    void setupOwnerComboBox();


    ///////////////////////////////////////////////////////////////////////
    /// UPDATE THE DATA STRUCTURE/DISPLAY                               ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void updateUnit(Unit unit);
    void selectUnit(int uniqueId);


    ///////////////////////////////////////////////////////////////////////
    /// SIGNALS                                                         ///
    ///////////////////////////////////////////////////////////////////////
signals:
    void unitChanged(Unit unit);


    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    Data * mData;
    int mSelectedUniqueId;
    Unit mCurrentUnit;

private:
    QLabel * mIcon;
    QLineEdit * mIdLineEdit;
    QLineEdit * mDisplayLineEdit;
    QComboBox * mOwner;
    QSlider * mLifeSlider;
    QLineEdit * mApp6LineEdit;
    QComboBox * mApp6Affiliation;
    QComboBox * mApp6Function;
    QComboBox * mApp6Size;

    QLabel * idLabel;
    QLabel * lifeLabel;
    QLabel * app6Label;

    bool mHackNoAdminPower;
};

#endif // UNITDETAIL_H
