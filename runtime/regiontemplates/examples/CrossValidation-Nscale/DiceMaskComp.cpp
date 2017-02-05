#include "DiceMaskComp.h"
#include <regiontemplates/comparativeanalysis/pixelcompare/PixelCompare.h>
#include <regiontemplates/comparativeanalysis/hadoopgis/predicates/DiceCoefficient.h>
#include <regiontemplates/comparativeanalysis/hadoopgis/predicates/JaccardIndex.h>
#include <regiontemplates/comparativeanalysis/hadoopgis/predicates/MaskIntersection.h>
#include <regiontemplates/comparativeanalysis/hadoopgis/predicates/DiceNotCoolCoefficient.h>

DiceMaskComp::DiceMaskComp() {
    this->setComponentName("DiceMaskComp");
    this->addInputOutputDataRegion("tile", "MASK", RTPipelineComponentBase::INPUT);
}

DiceMaskComp::~DiceMaskComp() {

}

int DiceMaskComp::run() {
    int parameterSegId = ((ArgumentInt *) this->getArgument(0))->getArgValue();

    RegionTemplate *inputRt = this->getRegionTemplateInstance("tile");

    float *diffPixels = (float *) malloc(2 * sizeof(float));
    diffPixels[0] = 0;
    diffPixels[1] = 0;
    this->setResultData((char *) diffPixels, 2 * sizeof(float));

    if (inputRt != NULL) {

        // Mask computed in segmentation using specific application parameter set
        DenseDataRegion2D *computed_mask = dynamic_cast<DenseDataRegion2D *>(inputRt->getDataRegion("MASK", "", 0,
                                                                                                    parameterSegId));
        // Mask used as a reference
        DenseDataRegion2D *reference_mask = dynamic_cast<DenseDataRegion2D *>(inputRt->getDataRegion("REF_MASK"));

        if (computed_mask != NULL && reference_mask != NULL) {
            // gambiarra
            diffPixels[0] = this->getId();
            // Create processing task
            TaskDiffMask *tDiffMask = new DiceCoefficient(computed_mask, reference_mask, diffPixels);

            this->executeTask(tDiffMask);
        } else {
            std::cout << "DiceMaskComp: did not find data regions: " << std::endl;
            inputRt->print();
        }
    } else {
        std::cout << "\tTASK dice mask: Did not find RT named tile" << std::endl;
    }

    return 0;
}

// Create the component factory
PipelineComponentBase *componentFactoryDF() {
    return new DiceMaskComp();
}

// register factory with the runtime system
bool registeredDF = PipelineComponentBase::ComponentFactory::componentRegister("DiceMaskComp", &componentFactoryDF);


