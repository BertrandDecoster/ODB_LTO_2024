#include "unitdetail.h"

#include "unit.h"
#include "data.h"
#include "mainwindow.h"

#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QLabel>
#include <QAbstractItemView>
#include <QDebug>
#include <QSlider>


///////////////////////////////////////////////////////////////////////
/// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
///////////////////////////////////////////////////////////////////////
///
UnitDetail::UnitDetail(Data *data, QWidget *parent):QWidget(parent),
    mData(data),
    mSelectedUniqueId(-1),
    mHackNoAdminPower(false)
{

    setupGUI();
    setupConnect();

    //    unitSelected(-1);
}

void UnitDetail::setupGUI()
{
    QLabel * iconLabel = new QLabel(QStringLiteral("Icon"), this);
    idLabel = new QLabel(QStringLiteral("Unique ID"), this);
    QLabel * displayLabel = new QLabel(QStringLiteral("Map name"), this);
    QLabel * ownerLabel = new QLabel(QStringLiteral("Owner"), this);
    lifeLabel = new QLabel(QStringLiteral("Life"), this);
    app6Label = new QLabel(QStringLiteral("APP-6"), this);
    QLabel * affiliationLabel = new QLabel(QStringLiteral("Affiliation"), this);
    QLabel * functionLabel = new QLabel(QStringLiteral("Function"), this);
    QLabel * sizeLabel = new QLabel(QStringLiteral("Size"), this);


    mIcon = new QLabel("No icon", this);
    mIdLineEdit = new QLineEdit(this);
    mDisplayLineEdit = new QLineEdit(this);
    mOwner = new QComboBox(this);
    mLifeSlider = new QSlider(Qt::Horizontal, this);
    mApp6LineEdit = new QLineEdit(this);
    mApp6Affiliation = new QComboBox(this);
    mApp6Function = new QComboBox(this);
    mApp6Size = new QComboBox(this);

    mIdLineEdit->setDisabled(true);
    mLifeSlider->setMinimum(0);
    mLifeSlider->setMaximum(100);
    mLifeSlider->setSingleStep(1);
    mLifeSlider->setTickPosition(QSlider::TicksBelow);
    mLifeSlider->setTickInterval(10);
    mLifeSlider->hide(); // HACK 2024
    mIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIcon->setMaximumHeight(128);
    mIcon->setMinimumHeight(64);
    //    mIdLineEdit->setDisabled(true);

    QGridLayout * layout = new QGridLayout(this);
    setLayout(layout);
    int row = 0;
    int col = 0;
    layout->addWidget(iconLabel, row, col);
    layout->addWidget(mIcon, row, col+1);
    // row++;
    // HACK 2024
    // layout->addWidget(idLabel, row, col);
    // layout->addWidget(mIdLineEdit, row, col+1);
    // row++;
    layout->addWidget(displayLabel, row, col+2);
    layout->addWidget(mDisplayLineEdit, row, col+3);
    // row++;

    // layout->addWidget(ownerLabel, row, col+4);
    // layout->addWidget(mOwner, row, col+5);
    // row++;
    // layout->addWidget(lifeLabel, row, col);
    // layout->addWidget(mLifeSlider, row, col+1);
    // row++;
    layout->addWidget(app6Label, row, col+4);
    layout->addWidget(mApp6LineEdit, row, col+5);
    row++;
    col = 0;
    layout->addWidget(affiliationLabel, row, col);
    layout->addWidget(mApp6Affiliation, row, col+1);
    // row++;
    layout->addWidget(functionLabel, row, col+2);
    layout->addWidget(mApp6Function, row, col+3);
    // row++;
    layout->addWidget(sizeLabel, row, col+4);
    layout->addWidget(mApp6Size, row, col+5);
    row++;

    // HACK 2024
    ownerLabel->hide();
    mOwner->hide();
    lifeLabel->hide();
    mLifeSlider->hide();
    idLabel->hide();
    mIdLineEdit->hide();
}

void UnitDetail::setupConnect()
{
    //    connect(mIdLineEdit, &QLineEdit::textEdited, [this](const QString & s){
    //        int id = s.toInt();
    //        if(!mData->units().contains(id)){
    //            idLabel->setStyleSheet("QLabel { color : black; }");
    //            mCurrentUnit.setUniqueId(id);
    //            emit unitChanged(mSelectedUniqueId, mCurrentUnit);
    //        }
    //        else{
    //            idLabel->setStyleSheet("QLabel { color : red; }");
    //        }
    //        mCurrentUnit.setUniqueId(id);
    //        emit unitChanged(mSelectedUniqueId, mCurrentUnit);
    //    });
    connect(mDisplayLineEdit, &QLineEdit::textEdited, [this](const QString & s){
        mCurrentUnit.setDisplayedText(s);
        emit unitChanged(mCurrentUnit);
    });
    connect(mOwner, QOverload<const QString &>::of(&QComboBox::activated), [this](const QString & s){
        mCurrentUnit.setOwner(s);
        qDebug() << "Unit is now " << mCurrentUnit;

        emit unitChanged(mCurrentUnit);
    });
    connect(mLifeSlider, &QSlider::valueChanged, [this](int value){
        if(static_cast<int>(mCurrentUnit.getLife()) == value) return;
        mCurrentUnit.setLife(value);
        lifeLabel->setText(QString("Life: ") + QString::number(value));
        emit unitChanged(mCurrentUnit);
    });
    connect(mApp6LineEdit, &QLineEdit::textEdited, [this](const QString & s){
        if(Unit::isApp6Legal(s)){
            app6Label->setStyleSheet("QLabel { color : black; }");
            mCurrentUnit.setApp6(s);
            emit unitChanged(mCurrentUnit);
        }
        else{
            app6Label->setStyleSheet("QLabel { color : red; }");
        }});

    connect(mApp6Affiliation, QOverload<const QString &>::of(&QComboBox::activated), [this](const QString & s){
        mCurrentUnit.setAffiliation(Unit::getAffiliation(s));
        emit unitChanged(mCurrentUnit);
    });
    connect(mApp6Function, QOverload<const QString &>::of(&QComboBox::activated), [this](const QString & s){
        mCurrentUnit.setFunction(Unit::getFunction(s));
        emit unitChanged(mCurrentUnit);
    });
    connect(mApp6Size, QOverload<const QString &>::of(&QComboBox::activated), [this](const QString & s){
        mCurrentUnit.setSize(Unit::getSize(s));
        emit unitChanged(mCurrentUnit);
    });
}

void UnitDetail::setupApp6ComboBox()
{
    mApp6Affiliation->clear();
    mApp6Function->clear();
    mApp6Size->clear();


    Unit * unit = mData->units()[mSelectedUniqueId];



    int h = 48;
    //    mApp6Affiliation->view()->setMinimumHeight(100);
    //    mApp6Affiliation->view()->setMinimumWidth(500);
    mApp6Affiliation->setMinimumHeight(h);
    mApp6Function->setMinimumHeight(h);
    mApp6Size->setMinimumHeight(h);

    mApp6Affiliation->setIconSize(QSize(h,h));
    mApp6Function->setIconSize(QSize(h,h));
    mApp6Size->setIconSize(QSize(h,h));


    QString app6Basis = QStringLiteral("S-GPU-------");
    app6Basis = Unit::setAffiliation(app6Basis, unit->affiliation());


    for(Unit::AFFILIATION affiliation : Unit::affiliations){
        QString app6 = Unit::setAffiliation(app6Basis, affiliation);
        if(Unit::isApp6Present(app6)){
            Unit tempUnit;
            tempUnit.setApp6(app6);
            mApp6Affiliation->addItem(tempUnit.getIcon(), Unit::getAffiliationName(affiliation));
            if(unit->affiliation() == affiliation){
                mApp6Affiliation->setCurrentIndex(mApp6Affiliation->count()-1);
            }
        }
    }

    for(Unit::FUNCTION function : Unit::functions){
        QString app6 = Unit::setFunction(app6Basis, function);
        if(Unit::isApp6Present(app6)){
            Unit tempUnit;
            tempUnit.setApp6(app6);
            mApp6Function->addItem(tempUnit.getIcon(), Unit::getFunctionName(function));
            if(unit->function() == function){
                mApp6Function->setCurrentIndex(mApp6Function->count()-1);
            }
        }
    }

    for(Unit::SIZE size: Unit::sizes){
        QString app6 = Unit::setSize(app6Basis, size);
        if(Unit::isApp6Present(app6)){
            Unit tempUnit;
            tempUnit.setApp6(app6);
            mApp6Size->addItem(tempUnit.getIcon(), Unit::getSizeName(size));
            if(unit->size() == size){
                mApp6Size->setCurrentIndex(mApp6Size->count()-1);
            }
        }
    }
}

void UnitDetail::setupOwnerComboBox()
{
    mOwner->clear();

    Unit * unit = mData->units()[mSelectedUniqueId];

    mOwner->addItem("");
    mOwner->setCurrentIndex(0);
    for(const QString & player : mData->getPlayerNames()){
        //        qDebug() << "Check owner " << player << " VS " << unit->owner();
        mOwner->addItem(player);
        if(unit->owner() == player){
            mOwner->setCurrentIndex(mOwner->count()-1);
        }
    }
}



///////////////////////////////////////////////////////////////////////
/// UPDATE THE DATA STRUCTURE/DISPLAY                               ///
///////////////////////////////////////////////////////////////////////
///
void UnitDetail::updateUnit(Unit unit)
{
    // Ignore changes about units that are not the one currently selected
    int uniqueId = unit.uniqueId();
    if(mSelectedUniqueId == uniqueId){
        selectUnit(uniqueId);
    }
}

void UnitDetail::selectUnit(int uniqueId)
{
    //    qDebug() << "ID " << uniqueId;

    // TODO : check if it's useful
    const QSignalBlocker blocker1(mIdLineEdit);
    const QSignalBlocker blocker2(mDisplayLineEdit);
    const QSignalBlocker blocker2b(mOwner);
    const QSignalBlocker blocker2c(mLifeSlider);
    const QSignalBlocker blocker3(mApp6LineEdit);
    const QSignalBlocker blocker4(mApp6Affiliation);
    const QSignalBlocker blocker5(mApp6Function);
    const QSignalBlocker blocker6(mApp6Size);

    auto units = mData->units();
    if(units.contains(uniqueId)){
        mSelectedUniqueId = uniqueId;


        Unit * unit = units[uniqueId];
        mCurrentUnit = Unit(*unit);
        setDisabled(false);

        MainWindow * mainWindow = dynamic_cast<MainWindow*>(parent()->parent()->parent()->parent()->parent());
        if(mainWindow){
            if(mainWindow->adminView()){
                setDisabled(false);
            }
            else{
                QString player = mainWindow->playerView();
                mCurrentUnit = mData->getPlayerVisionOfUnit(player, uniqueId);
                setDisabled(true);
                mOwner->hide();
            }
        }
        else{
            qDebug() << "FUCK";
        }

        //        Unit * unit = units[uniqueId];
        //        mCurrentUnit = Unit(*unit);
        mIcon->setText("");
        mIcon->setPixmap(mCurrentUnit.getIcon().pixmap(mIcon->size()));

        mIdLineEdit->setText(QString::number(mCurrentUnit.uniqueId()));
        mDisplayLineEdit->setText(mCurrentUnit.displayedText());
        int life = static_cast<int>(mCurrentUnit.getLife());
        qDebug() << "Asked to set life to " << life;
        mLifeSlider->setValue(life);
        lifeLabel->setText(QString("Life: ") + QString::number(life));
        mApp6LineEdit->setText(mCurrentUnit.app6());
        setupApp6ComboBox();
        setupOwnerComboBox();


    }
    else{
        mSelectedUniqueId = -1;
        mCurrentUnit = Unit();
        mIcon->setText("No icon");
        mIcon->setPixmap(QPixmap());
        mIdLineEdit->clear();
        mDisplayLineEdit->clear();
        lifeLabel->setText(QString("Life"));
        mApp6LineEdit->clear();
        setupApp6ComboBox();    // TODO check

        setDisabled(true);
    }
}

