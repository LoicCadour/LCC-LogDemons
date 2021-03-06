/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "LCCLogDemons.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractProcessFactory.h>

// /////////////////////////////////////////////////////////////////
//
// /////////////////////////////////////////////////////////////////

#include "itkImageRegistrationMethod.h"

#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"


#include "time.h"

#include <rpiLCClogDemons.hxx>
#include <rpiCommonTools.hxx>

// /////////////////////////////////////////////////////////////////
// LCCLogDemonsPrivate
// /////////////////////////////////////////////////////////////////
typedef itk::Image< float, 3 >  RegImageType;
typedef double TransformScalarType;

class LCCLogDemonsPrivate
{
public:
    LCCLogDemons * proc;
    template <class PixelType>
    int update();
    template <typename PixelType>
    bool writeTransform(const QString& file);
    
    rpi::LCClogDemons< RegImageType, RegImageType, float > * registrationMethod;
    rpi::LCClogDemons< RegImageType, RegImageType, float >::UpdateRule updateRule;
    std::vector<unsigned int> iterations;

    bool verbose;
};

// /////////////////////////////////////////////////////////////////
// LCCLogDemons
// /////////////////////////////////////////////////////////////////

LCCLogDemons::LCCLogDemons() : itkProcessRegistration(), d(new LCCLogDemonsPrivate)
{
    d->proc = this;
    d->registrationMethod = NULL;
}

LCCLogDemons::~LCCLogDemons()
{
    d->proc = NULL;

    
    if (d->registrationMethod)
        delete d->registrationMethod;

    d->registrationMethod = NULL;
    
    delete d;
    d = 0;
}

bool LCCLogDemons::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("LCCLogDemons",
                                                                      createLCCLogDemons);
}

void LCCLogDemons::setUpdateRule(int rule)
{
    d->updateRule = (rpi::LCClogDemons< RegImageType, RegImageType, float >::UpdateRule)rule;
}

void LCCLogDemons::setVerbosity(bool verbose)
{
    d->verbose = verbose;
}

void LCCLogDemons::setIterations(std::vector<unsigned int> iterations)
{
    d->iterations = iterations;
}

QString LCCLogDemons::description() const
{
    return "LCCLogDemons";
}



// /////////////////////////////////////////////////////////////////
// Templated Version of update
// /////////////////////////////////////////////////////////////////


template <typename PixelType>
int LCCLogDemonsPrivate::update()
{
    registrationMethod = new rpi::LCClogDemons<RegImageType,RegImageType,float> ();
    
    registrationMethod->SetFixedImage((const RegImageType*) proc->fixedImage().GetPointer());
    registrationMethod->SetMovingImage((const RegImageType*) proc->movingImages()[0].GetPointer());
    
    registrationMethod->SetUpdateRule(updateRule);
    registrationMethod->SetVerbosity(verbose);
    registrationMethod->SetNumberOfIterations(iterations);
    
    // Run the registration
    time_t t1 = clock();
    try {
        registrationMethod->StartRegistration();
    }
    catch( std::exception & err )
    {
        qDebug() << "ExceptionObject caught ! (startRegistration)" << err.what();
        return 1;
    }
    
    time_t t2 = clock();
    
    qDebug() << "Elasped time: " << (double)(t2-t1)/(double)CLOCKS_PER_SEC;
    
    typedef itk::ResampleImageFilter< RegImageType,RegImageType, float>    ResampleFilterType;
    typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
    resampler->SetTransform(registrationMethod->GetTransformation());
    resampler->SetInput((const RegImageType*)proc->movingImages()[0].GetPointer());
    resampler->SetSize( proc->fixedImage()->GetLargestPossibleRegion().GetSize() );
    resampler->SetOutputOrigin( proc->fixedImage()->GetOrigin() );
    resampler->SetOutputSpacing( proc->fixedImage()->GetSpacing() );
    resampler->SetOutputDirection( proc->fixedImage()->GetDirection() );
    resampler->SetDefaultPixelValue( 0 );

    
    try {
        resampler->Update();
    }
    catch (itk::ExceptionObject &e) {
        qDebug() << e.GetDescription();
        return 1;
    }
    
    itk::ImageBase<3>::Pointer result = resampler->GetOutput();
    result->DisconnectPipeline();
    
    if (proc->output())
        proc->output()->setData (result);
    return 0;
}

int LCCLogDemons::update(itkProcessRegistration::ImageType imgType)
{
    if(fixedImage().IsNull() || movingImages()[0].IsNull())
        return 1;

    return d->update<float>();
}


template <typename PixelType>
bool LCCLogDemonsPrivate::writeTransform(const QString& file)
{

    
    if (registrationMethod)
    {
        try{
            rpi::writeDisplacementFieldTransformation<float, 3>(registrationMethod->GetTransformation(),
                                                                file.toStdString());
        }
        catch (std::exception)
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
    
}

bool LCCLogDemons::writeTransform(const QString& file)
{
    if(d->registrationMethod == NULL)
        return 1;
    
    return d->writeTransform<float>(file);
}

itk::Transform<double,3,3>::Pointer LCCLogDemons::getTransform()
{
    return NULL;
}

QString LCCLogDemons::getTitleAndParameters()
{
    return QString();
}

// /////////////////////////////////////////////////////////////////
// Type instanciation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createLCCLogDemons()
{
    return new LCCLogDemons;
}
