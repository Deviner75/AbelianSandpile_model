#include "sandDunes.h"

#include <GU/GU_Detail.h>
#include <GA/GA_Handle.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <SYS/SYS_Math.h>
#include <GEO/GEO_PointTree.h>
#include <UT/UT_ValArray.h>
#include <vector>


void
newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(new OP_Operator(
		"sandunes",
		"sandDunes ",
		sandDunes::myConstructor,
		sandDunes::myTemplateList,
		1,
		1,
		0));
}

static PRM_Name iterName("iters", "Iterations");
static PRM_Name massName("mass", "Mass");
static PRM_Name thrName("threshold", "Threshold");

static PRM_Default iterDefault(1);
static PRM_Default massDefault(0.05);
static PRM_Default trhDefault(0.09);

PRM_Template
sandDunes::myTemplateList[] = {
	PRM_Template(PRM_INT_J,  1, &iterName, &iterDefault, 0),
	PRM_Template(PRM_FLT_J,  1, &massName, &massDefault, 0),
	PRM_Template(PRM_FLT_J,  1, &thrName, &trhDefault, 0),
	PRM_Template(),
};


OP_Node *
sandDunes::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	return new sandDunes(net, name, op);
}

sandDunes::sandDunes(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
}

sandDunes::~sandDunes()
{
}

OP_ERROR
sandDunes::cookMySop(OP_Context &context)
{
	fpreal now = context.getTime();

	OP_AutoLockInputs inputs(this);
	if (inputs.lock(context) >= UT_ERROR_ABORT)
		return error();

	duplicateSource(0, context);

	int iterations = ITERATIONS(now);
	float mass = GETMASS(now);
	float trhs = GETTHRS(now);


	GA_ROHandleIA _npts(gdp, GA_ATTRIB_POINT, "__nearpoints__"); // get nearpoints
	GA_RWHandleF _h(gdp, GA_ATTRIB_POINT, "h"); // get height
	if (_h.isValid() && _npts.isValid()) // check if both exist
	{
		for (int i = 0; i < iterations; i++) 
		{
			for (GA_Iterator it(gdp->getPointRange()); !it.atEnd(); ++it)
			{
				GA_Offset offset = *it; // pt offset
				float h = _h.get(offset); // pt height
				UT_Int32Array npts;
				_npts.get(offset, npts);
				int randIdx = rand() % npts.size();
				GA_Offset nptOffset = gdp->pointOffset(npts[randIdx]); // near point offset
				float nptH = _h.get(nptOffset); // near point h
				if ((h - nptH) >= trhs) 
				{
					h -= mass;
					nptH += mass;
					_h.set(nptOffset, h);
					_h.set(offset, nptH);
				}
				//std::cout << "index: " << index << " h: " << h << "_npts " << nptIndex << " nptsH " << nptH << std::endl;
			}
		}
	}
	else
		std::cerr << " h or __nearpoints__  attribute is lost !!" << std::endl;
	return error();
}