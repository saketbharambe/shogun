/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2008 Soeren Sonnenburg
 * Written (W) 1999-2008 Gunnar Raetsch
 * Copyright (C) 1999-2008 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "lib/config.h"

#ifndef HAVE_SWIG
#include "lib/io.h"

#include "interface/SGInterface.h"
#include "guilib/GUIPluginEstimate.h"

#include "features/StringFeatures.h"


CGUIPluginEstimate::CGUIPluginEstimate(CSGInterface* ui_)
: CSGObject(), ui(ui_), estimator(NULL),
	pos_pseudo(1e-10), neg_pseudo(1e-10)
{
}

CGUIPluginEstimate::~CGUIPluginEstimate()
{
	delete estimator;
}

bool CGUIPluginEstimate::new_estimator(DREAL pos, DREAL neg)
{
	delete estimator;
	estimator=new CPluginEstimate(pos, neg);

	if (!estimator)
		SG_ERROR("Could not create new plugin estimator, pos_pseudo %f, neg_pseudo %f\n", pos_pseudo, neg_pseudo);
	else
		SG_INFO("Created new plugin estimator (%p), pos_pseudo %f, neg_pseudo %f\n", estimator, pos_pseudo, neg_pseudo);

	return true;
}

bool CGUIPluginEstimate::train()
{
	CLabels* trainlabels=ui->ui_labels.get_train_labels();
	CStringFeatures<WORD>* trainfeatures=(CStringFeatures<WORD>*) ui->ui_features.get_train_features();
	bool result=false;

	if (!trainlabels)
		SG_ERROR("No labels available.\n");

	if (!trainfeatures)
		SG_ERROR("No features available.\n");

	ASSERT(trainfeatures->get_feature_type()==F_WORD);

	estimator->set_features(trainfeatures);
	estimator->set_labels(trainlabels);
	if (estimator)
		result=estimator->train();
	else
		SG_ERROR("No estimator available.\n");

	return result;
}

bool CGUIPluginEstimate::test(CHAR* filename_out, CHAR* filename_roc)
{
	FILE* file_out=stdout;
	FILE* file_roc=NULL;

	if (!estimator)
		SG_ERROR("No estimator available.\n");

	if (!estimator->check_models())
		SG_ERROR("No models assigned.\n");

	CLabels* testlabels=ui->ui_labels.get_test_labels();
	if (!testlabels)
		SG_ERROR("No test labels available.\n");

	CFeatures* testfeatures=ui->ui_features.get_test_features();
	if (!testfeatures || testfeatures->get_feature_class()!=C_SIMPLE ||
		testfeatures->get_feature_type()!=F_WORD)
		SG_ERROR("No test features of type WORD available.\n");

	if (filename_out)
	{
		file_out=fopen(filename_out, "w");

		if (!file_out)
			SG_ERROR("Could not open file %s.\n", filename_out);

		if (filename_roc)
		{
			file_roc=fopen(filename_roc, "w");
			if (!file_roc)
				SG_ERROR("Could not open ROC file %s\n", filename_roc);
		}
	}

	SG_INFO("Starting estimator testing.\n");
	estimator->set_features((CStringFeatures<WORD>*) testfeatures);
	INT len=0;
	DREAL* output=estimator->classify()->get_labels(len);

	INT total=testfeatures->get_num_vectors();
	INT* label=testlabels->get_int_labels(len);

	SG_DEBUG("out !!! %ld %ld.\n", total, len);
	ASSERT(label);
	ASSERT(len==total);

	ui->ui_math.evaluate_results(output, label, total, file_out, file_roc);

	if (file_roc)
		fclose(file_roc);
	if (file_out && file_out!=stdout)
		fclose(file_out);

	delete[] output;
	delete[] label;
	return true;
}

bool CGUIPluginEstimate::load(CHAR* param)
{
  bool result=false;
  return result;
}

bool CGUIPluginEstimate::save(CHAR* param)
{
  bool result=false;
  return result;
}

CLabels* CGUIPluginEstimate::classify(CLabels* output)
{
	CFeatures* testfeatures=ui->ui_features.get_test_features();

	if (!estimator)
	{
		SG_ERROR( "no estimator available") ;
		return 0;
	}

	if (!testfeatures)
	{
		SG_ERROR( "no test features available") ;
		return 0;
	}

	estimator->set_features((CStringFeatures<WORD>*) testfeatures);

	return estimator->classify(output);
}

DREAL CGUIPluginEstimate::classify_example(INT idx)
{
	CFeatures* testfeatures=ui->ui_features.get_test_features();

	if (!estimator)
	{
		SG_ERROR( "no estimator available") ;
		return 0;
	}

	if (!testfeatures)
	{
		SG_ERROR( "no test features available") ;
		return 0;
	}

	estimator->set_features((CStringFeatures<WORD>*) testfeatures);

	return estimator->classify_example(idx);
}
#endif
