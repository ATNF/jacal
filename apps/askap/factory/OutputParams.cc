/// @file SoadNE.cc
///
/// @abstract
/// Derived from DaliugeApplication
/// @ details
/// IMplements an ASKAPSoft solver. This essentially takes a NormalEquation and generates a
/// a set of "params" usually via a minor cycle deconvolution.
///
// for logging
#define ASKAP_PACKAGE_NAME "OutputParams"
#include <string>
/// askap namespace
namespace askap {
/// @return version of the package
    std::string getAskapPackageVersion_OutputParams() {
        return std::string("OutputParams");
    }


} // namespace askap

/// The version of the package
#define ASKAP_PACKAGE_VERSION askap::getAskapPackageVersion_OutputParams()

#include <iostream>
#include <vector>



#include <daliuge/DaliugeApplication.h>
#include <factory/OutputParams.h>
#include <factory/NEUtils.h>

// LOFAR ParameterSet
#include <Common/ParameterSet.h>
// LOFAR Blob
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
// ASKAP Logger

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

// Data accessors
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>

// params helpers
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageParamsHelper.h>
#include <measurementequation/ImageFFTEquation.h>
#include <parallel/GroupVisAggregator.h>


#include <gridding/IVisGridder.h>
#include <gridding/VisGridderFactory.h>

// Solver stuff

#include <measurementequation/ImageSolverFactory.h>



#include <fitting/Params.h>
#include <fitting/Solver.h>

#include <string.h>
#include <sys/time.h>




namespace askap {



    OutputParams::OutputParams(dlg_app_info *raw_app) :
        DaliugeApplication(raw_app)
    {
        //ASKAP_LOGGER(locallogger,"\t OutputParams -  default contructor\n");
        std::cout << "OutputParams -  constructor" << std::endl;

    }


    OutputParams::~OutputParams() {
        //ASKAP_LOGGER(locallogger,"\t OutputParams -  default destructor\n");
        std::cout << "OutputParams -  default destructor" << std::endl;
    }

    DaliugeApplication::ShPtr OutputParams::createDaliugeApplication(dlg_app_info *raw_app)
    {
        // ASKAP_LOGGER(locallogger, ".create");
        std::cout << "createDaliugeApplication - Instantiating OutputParams" << std::endl;
        // ASKAPLOG_INFO_STR(locallogger,"createDaliugeApplication - Instantiating OutputParams");
        OutputParams::ShPtr ptr;

        // We need to pull all the parameters out of the parset - and set
        // all the private variables required to define the beam


        ptr.reset( new OutputParams(raw_app));

        std::cout << "createDaliugeApplication - Created OutputParams DaliugeApplication instance " << std::endl;
        return ptr;

    }
    int OutputParams::init(const char ***arguments) {



        while (1) {

            const char **param = *arguments;

            // Sentinel
            if (param == NULL) {
                break;
            }
            // if (strcmp(param[0], "config") == 0) {
            //    config_key = strdup(param[1]);
            // }
            // arguments++;
        }

        return 0;
    }

    int OutputParams::run() {

        // Lets get the key-value-parset
        // ASKAPLOG_INIT("");
        ASKAP_LOGGER(logger, ".run");

        // lets open the input and read it
        char buf[64*1024];
        size_t n_read = input("Config").read(buf, 64*1024);

        LOFAR::ParameterSet parset(true);
        parset.adoptBuffer(buf);

        this->itsParset = parset.makeSubset("Cimager.");

        try {
            this->itsParset = NEUtils::addMissingParameters(this->itsParset);
        }
        catch (std::runtime_error)
        {
            return -1;
        }

        // Actually there should be no model on input .... it should always be empty
        // it is my job to fill it.
        this->itsModel.reset(new scimath::Params());
        NEUtils::receiveParams(itsModel, input("Model"));

        // askap::synthesis::SynthesisParamsHelper::setUpImages(itsModel,
        //                          this->itsParset.makeSubset("Images."));


        synthesis::SynthesisParamsHelper::setUpImageHandler(itsParset);

        vector<string> images=itsModel->names();

        for (vector<string>::const_iterator it=images.begin(); it !=images.end(); it++) {
          std::cout << "Model contains "<< *it << std::endl;
          if (it->find("image") == 0) {
            const synthesis::ImageParamsHelper iph(*it);
            synthesis::SynthesisParamsHelper::saveImageParameter(*itsModel, *it, *it);
          }
          if (it->find("residual") == 0) {
            const synthesis::ImageParamsHelper iph(*it);
            synthesis::SynthesisParamsHelper::saveImageParameter(*itsModel, *it, *it);
          }

        }


        return 0;
    }


    void OutputParams::data_written(const char *uid, const char *data, size_t n) {
        dlg_app_running();
    }

    void OutputParams::drop_completed(const char *uid, drop_status status) {
        dlg_app_done(APP_FINISHED);
    }


} // namespace
