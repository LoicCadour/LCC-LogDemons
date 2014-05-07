/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "LCCLogDemons.h"
#include "LCCLogDemonsToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>
#include <medRunnableProcess.h>
#include <medJobManager.h>

#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medRegistrationSelectorToolBox.h>
#include <medProgressionStack.h>
#include <medGroupBox.h>

#include <rpiCommonTools.hxx>

class LCCLogDemonsToolBoxPrivate
{
public:

    QWidget *logWidget, *LCCWidget, *commonWidget;
    QLineEdit * iterationsLine;
    QComboBox * updateRuleComboBox, * gradientTypeComboBox;
    QSpinBox * bchExpansionSpinBox;
    QDoubleSpinBox *sigmaISpinBox, *similaritySigmaSpinBox, *updateFieldSigmaSpinBox,
        *velocityFieldSigmaSpinBox, *maxStepLengthSpinBox;
    QCheckBox * boundaryCheckBox, *histoMatchingCheckBox;
    int updateRule;

    medProgressionStack * progression_stack;
};

LCCLogDemonsToolBox::LCCLogDemonsToolBox(QWidget *parent) : medRegistrationAbstractToolBox(parent), d(new LCCLogDemonsToolBoxPrivate)
{
    this->setTitle("LCCLogDemons");
    
    QWidget *widget = new QWidget(this);

    QVBoxLayout * layout = new QVBoxLayout();
    
    //QFormLayout *layout = new QFormLayout(widget);
    //
    //medGroupBox * advancedBox = new medGroupBox(this);
    //advancedBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //advancedBox->setTitle(tr("Advanced Parameters"));
    //advancedBox->setCollapsible(true);
    //advancedBox->setCheckable(true);
    //advancedBox->setChecked(false);
    //advancedBox->setStyleSheet("medGroupBox {margin: 10px;}");
    //QWidget * advancedWidgets = new QWidget(advancedBox);
    //QFormLayout *advancedformLayout = new QFormLayout(advancedWidgets);
    //QVBoxLayout * vlayout = new QVBoxLayout(advancedBox);
    //vlayout->addWidget(advancedWidgets);

    //// Standard parameters
    //d->doubleIterations = new QCheckBox();
    //layout->addRow(new QLabel(tr("Double "),this),d->doubleIterations);

    d->iterationsLine = new QLineEdit();
    d->iterationsLine->setText("15x10x5");
    d->iterationsLine->setToolTip(tr("Each number of iteration per level must be separated by \"x\". From coarser to finest levels"));
    QLabel * iterationsLabel = new QLabel("Iterations per level  of res.");
    QHBoxLayout * iterationsLayout = new QHBoxLayout();
    iterationsLayout->addWidget(iterationsLabel);
    iterationsLayout->addWidget(d->iterationsLine);
    //layout->addRow(new QLabel(tr("Iterations per level of res."),this),d->iterationsBox);


    d->updateRuleComboBox =  new QComboBox();
    d->updateRuleComboBox->insertItem(0, "Choose update rule");
    d->updateRuleComboBox->insertItem(1, "Log");
    d->updateRuleComboBox->insertItem(2, "LCC");
    connect ( d->updateRuleComboBox, SIGNAL ( activated ( int ) ), this, SLOT ( chooseUpdateRule ( int ) ) );

    //Parameters for Log   

    QLabel * maxStepLengthLabel = new QLabel("Maximum length of an update vector:");
    d->maxStepLengthSpinBox = new QDoubleSpinBox();
    d->maxStepLengthSpinBox->setValue(2.0);
    QHBoxLayout * maxStepLengthLayout = new QHBoxLayout();
    maxStepLengthLayout->addWidget(maxStepLengthLabel);
    maxStepLengthLayout->addWidget(d->maxStepLengthSpinBox);

    QLabel * gradientTypeLabel = new QLabel("Gradient Type:");
    d->gradientTypeComboBox = new QComboBox();
    d->gradientTypeComboBox->insertItem(0, "Symmetrized");
    d->gradientTypeComboBox->insertItem(1, "Fixed image");
    d->gradientTypeComboBox->insertItem(2, "Warped moving image");
    d->gradientTypeComboBox->insertItem(3, "Mapped moving image");
    QHBoxLayout * gradientTypeLayout = new QHBoxLayout();
    gradientTypeLayout->addWidget(gradientTypeLabel);
    gradientTypeLayout->addWidget(d->gradientTypeComboBox);

    QLabel * histoMatchingLabel = new QLabel("Use Histogram Matching:");
    d->histoMatchingCheckBox = new QCheckBox();
    d->histoMatchingCheckBox->setChecked(true);
    QHBoxLayout * histoMatchingLayout  = new QHBoxLayout();
    histoMatchingLayout->addWidget(histoMatchingLabel);
    histoMatchingLayout->addWidget(d->histoMatchingCheckBox);


    //Parameters for LCC
    
    QLabel * boundaryLabel = new QLabel("Boundary:");
    d->boundaryCheckBox = new QCheckBox();
    d->boundaryCheckBox->setChecked(true);
    QHBoxLayout * boundaryLayout  = new QHBoxLayout();
    boundaryLayout->addWidget(boundaryLabel);
    boundaryLayout->addWidget(d->boundaryCheckBox);

    QLabel * sigmaILabel = new QLabel("Sigma I:");
    d->sigmaISpinBox = new QDoubleSpinBox();
    d->sigmaISpinBox->setValue(0.2);
    QHBoxLayout * sigmaILayout = new QHBoxLayout();
    sigmaILayout->addWidget(sigmaILabel);
    sigmaILayout->addWidget(d->sigmaISpinBox);

    QLabel * similarityLabel = new QLabel("Similarity Criteria Sigma:");
    d->similaritySigmaSpinBox = new QDoubleSpinBox();
    d->similaritySigmaSpinBox->setValue(3.0);
    QHBoxLayout * similarityLayout = new QHBoxLayout();
    similarityLayout->addWidget(similarityLabel);
    similarityLayout->addWidget(d->similaritySigmaSpinBox);

    //Common parameters

    QLabel * updateFieldSigmaLabel = new QLabel("Update Field Sigma:");
    d->updateFieldSigmaSpinBox = new QDoubleSpinBox();
    d->updateFieldSigmaSpinBox->setValue(0.0);
    QHBoxLayout * updateFieldSigmaLayout = new QHBoxLayout();
    updateFieldSigmaLayout->addWidget(updateFieldSigmaLabel);
    updateFieldSigmaLayout->addWidget(d->updateFieldSigmaSpinBox);

    QLabel * velocityFieldSigmaLabel = new QLabel("Velocity Field Sigma:");
    d->velocityFieldSigmaSpinBox = new QDoubleSpinBox();
    d->velocityFieldSigmaSpinBox->setValue(1.5);
    QHBoxLayout * velocityFieldSigmaLayout = new QHBoxLayout();
    velocityFieldSigmaLayout->addWidget(velocityFieldSigmaLabel);
    velocityFieldSigmaLayout->addWidget(d->velocityFieldSigmaSpinBox);

    QLabel * bchExpansionLabel = new QLabel("BCH Expansion:");
    d->bchExpansionSpinBox = new QSpinBox();
    d->bchExpansionSpinBox->setValue(2);
    QHBoxLayout * bchExpansionLayout = new QHBoxLayout();
    bchExpansionLayout->addWidget(bchExpansionLabel);
    bchExpansionLayout->addWidget(d->bchExpansionSpinBox);

    QPushButton *runButton = new QPushButton(tr("Run"), this);

    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);

    // Layouts

    QVBoxLayout * logLayout = new QVBoxLayout();
    logLayout->addLayout(maxStepLengthLayout);
    logLayout->addLayout(gradientTypeLayout);
    logLayout->addLayout(histoMatchingLayout);
    d->logWidget = new QWidget(this);
    d->logWidget->setLayout(logLayout);

    QVBoxLayout * LCCLayout = new QVBoxLayout();
    LCCLayout->addLayout(boundaryLayout);
    LCCLayout->addLayout(sigmaILayout);
    LCCLayout->addLayout(similarityLayout);
    d->LCCWidget = new QWidget(this);
    d->LCCWidget->setLayout(LCCLayout);

    QVBoxLayout * commonLayout = new QVBoxLayout();
    commonLayout->addLayout(updateFieldSigmaLayout);
    commonLayout->addLayout(velocityFieldSigmaLayout);
    commonLayout->addLayout(bchExpansionLayout);
    d->commonWidget = new QWidget(this);
    d->commonWidget->setLayout(commonLayout);

    //Main layout
    layout->addLayout(iterationsLayout);
    layout->addWidget(d->updateRuleComboBox);
    layout->addWidget(d->LCCWidget);
    layout->addWidget(d->logWidget);
    layout->addWidget(d->commonWidget);
    layout->addWidget(runButton);
    layout->addLayout(progressStackLayout);

    widget->setLayout(layout);
    this->addWidget(widget);
    
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
    chooseUpdateRule(0);
}

LCCLogDemonsToolBox::~LCCLogDemonsToolBox()
{
    delete d;
    
    d = NULL;
}

bool LCCLogDemonsToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<LCCLogDemonsToolBox>("LCCLogDemonsToolBox",
                               tr("LCC Log Demons"),
                               tr("short tooltip description"),
                               QStringList() << "registration");
}

void LCCLogDemonsToolBox::chooseUpdateRule(int choice)
{
    d->LCCWidget->hide();
    d->logWidget->hide();
    d->commonWidget->hide();

    if(choice == 0)
    {
    }
    else if (choice == 1)
    {
        d->logWidget->show();
        d->commonWidget->show();
        d->updateRule = 1;
    }
    else if (choice == 2)
    {
        d->LCCWidget->show();
        d->commonWidget->show();
        d->updateRule = 2;
    }
    else
        return;
}

void LCCLogDemonsToolBox::run()
{
    
    if(!this->parentToolBox())
        return;
    medRegistrationSelectorToolBox * parentTB = this->parentToolBox();
    dtkSmartPointer <dtkAbstractProcess> process;
    
    if (this->parentToolBox()->process() &&
        (parentTB->process()->identifier() == "LCCLogDemons"))
    {
        process = parentTB->process();
        
    }
    else
    {
        process = dtkAbstractProcessFactory::instance()->createSmartPointer("LCCLogDemons");
        parentTB->setProcess(process);
    }
    dtkAbstractData *fixedData = parentTB->fixedData();
    dtkAbstractData *movingData = parentTB->movingData();
    
    
    if (!fixedData || !movingData)
        return;
    
    
    LCCLogDemons *process_Registration = dynamic_cast<LCCLogDemons *>(process.data());
    if (!process_Registration)
    {
        qWarning() << "registration process doesn't exist" ;
        return;
    }
    process_Registration->setUpdateRule(d->updateRule);
    process_Registration->setVerbosity(true);

    process_Registration->setMaximumUpdateStepLength(d->maxStepLengthSpinBox->value());
    process_Registration->setGradientType(d->gradientTypeComboBox->currentIndex());
    process_Registration->setUseHistogramMatching(d->histoMatchingCheckBox->isChecked());

    process_Registration->setBoundaryCheck(d->boundaryCheckBox->isChecked());
    process_Registration->setSigmaI(d->sigmaISpinBox->value());
    process_Registration->setSimilarityCriteriaSigma(d->similaritySigmaSpinBox->value());
    process_Registration->setUpdateFieldSigma(d->updateFieldSigmaSpinBox->value());
    process_Registration->setVelocityFieldSigma(d->velocityFieldSigmaSpinBox->value());
    process_Registration->setNumberOfTermsBCHExpansion((unsigned int)d->bchExpansionSpinBox->value());
    process_Registration->useMask(false); //Should we allow the use of mask ?

    try {
        process_Registration->setNumberOfIterations(
                    rpi::StringToVector<unsigned int>(
                        d->iterationsLine->text().toStdString()));
    }
    catch ( std::exception & )
    {
        qDebug() << "wrong iteration format";
        return;
    }
    
    process->setInput(fixedData,  0);
    process->setInput(movingData, 1);
    
    medRunnableProcess *runProcess = new medRunnableProcess;
    runProcess->setProcess (process);
    
    d->progression_stack->addJobItem(runProcess, tr("Progress:"));
    //If there is no observer to track the progression,
    //make the progress bar spin:
    //d->progression_stack->setActive(runProcess,true);
    
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SIGNAL (success ()));
    connect (runProcess, SIGNAL (failure  (QObject*)),  this, SIGNAL (failure ()));
    connect (runProcess, SIGNAL (cancelled (QObject*)), this, SIGNAL (failure ()));
    //First have the moving progress bar,
    //and then display the remaining % when known
    connect (runProcess, SIGNAL(activate(QObject*,bool)),
             d->progression_stack, SLOT(setActive(QObject*,bool)));
    
    medJobManager::instance()->registerJobItem(runProcess);
    QThreadPool::globalInstance()->start(dynamic_cast<QRunnable*>(runProcess));
}
