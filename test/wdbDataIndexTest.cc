/*
    fimex

    Copyright (C) 2011 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: post@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/

#include <wdb/DataIndex.h>
#include <wdb/CdmNameTranslator.h>
#include <fimex/CDM.h>
#include <iostream>
#include <algorithm>
#include <boost/assign.hpp>


#include "../config.h"
#include <boost/version.hpp>
#if defined(HAVE_BOOST_UNIT_TEST_FRAMEWORK) && (BOOST_VERSION >= 103400)

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE WDB Interface Test
#include <boost/test/unit_test.hpp>

using namespace MetNoFimex;

/// Do not add any non-static fields to this class; they will disappear in tests
class TestingGridData : public wdb::GridData
{
	static boost::posix_time::ptime t(const std::string & time)
	{
		return boost::posix_time::time_from_string(time);
	}
public:
	static const wdb::Parameter defaultParameter;
	static const wdb::Level defaultLevel;
	static const std::string defaultTime;

	TestingGridData(const wdb::Level & lvl, const std::string & time = defaultTime) :
		wdb::GridData(defaultParameter, lvl, 0, t(time), 0)
	{}

	TestingGridData(const wdb::Parameter & parameter, const std::string & time = defaultTime) :
		wdb::GridData(parameter, defaultLevel, 0, t(time), 0)
	{}

	explicit TestingGridData(const std::string & time) :
		wdb::GridData(defaultParameter, defaultLevel, 0, t(time), 0)
	{}

	explicit TestingGridData(int dataVersion, const std::string & time = defaultTime) :
		wdb::GridData(defaultParameter, defaultLevel, dataVersion, t(time), 0)
	{}

	static std::string cdmId()
	{
		return "air_temperature";
	}
};
const wdb::Parameter TestingGridData::defaultParameter("air temperature", "C");
const wdb::Level TestingGridData::defaultLevel("distance above ground", "m", 0, 0);
const std::string TestingGridData::defaultTime = "2011-03-18 06:00:00";

wdb::CdmNameTranslator tr;

struct same_entity
{
	const std::string name_;
	same_entity(const std::string & name) : name_(name) {}

	bool operator() (const CDMNamedEntity & entity)
	{
		return entity.getName() == name_;
	}
};


BOOST_AUTO_TEST_SUITE(DataIndexTest)

BOOST_AUTO_TEST_CASE(setsBaseDimensions)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 1, 1)));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("latitude")) != dims.end());
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("longitude")) != dims.end());
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("time")) != dims.end());
	BOOST_CHECK_EQUAL(3, dims.size());
}

BOOST_AUTO_TEST_CASE(setsDimensionSizes)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 1, 1)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2)));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("latitude")) != dims.end());
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("longitude")) != dims.end());
	BOOST_CHECK(std::find_if(dims.begin(), dims.end(), same_entity("time")) != dims.end());

	BOOST_CHECK_EQUAL(4, dims.size());

	CDM::DimVec::const_iterator lvlDimension = std::find_if(dims.begin(), dims.end(), same_entity("lvl"));
	BOOST_REQUIRE(lvlDimension != dims.end());
	BOOST_CHECK_EQUAL(3, lvlDimension->getLength());
}

BOOST_AUTO_TEST_CASE(setsCorrectTimeDimensionSizeWithOneTimeStep)
{
	// since there is only one time here, we don't add time as a dimension.
	// but we still add time as an unlimited dimension (of size 0).
	//
	// (todo: is there any problems with this?)

	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2)));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();

	CDM::DimVec::const_iterator timeDimension = std::find_if(dims.begin(), dims.end(), same_entity("time"));
	BOOST_REQUIRE(timeDimension != dims.end());
	BOOST_CHECK_EQUAL(0, timeDimension->getLength());
}

BOOST_AUTO_TEST_CASE(setsCorrectTimeDimensionSize)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 06:00:00"));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 07:00:00"));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0), "2010-03-18 07:00:00"));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();

	CDM::DimVec::const_iterator timeDimension = std::find_if(dims.begin(), dims.end(), same_entity("time"));
	BOOST_REQUIRE(timeDimension != dims.end());
	BOOST_CHECK_EQUAL(2, timeDimension->getLength());
}

BOOST_AUTO_TEST_CASE(picksUpAllTimesFromSameParameter)
{
	// All times for varying levels should be picked up

	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 06:00:00"));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 07:00:00"));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0), "2000-01-01 00:00:00"));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();

	CDM::DimVec::const_iterator timeDimension = std::find_if(dims.begin(), dims.end(), same_entity("time"));
	BOOST_REQUIRE(timeDimension != dims.end());
	BOOST_CHECK_EQUAL(3, timeDimension->getLength());
}

BOOST_AUTO_TEST_CASE(ignoresIrrelevantTimeDimensions)
{
	// Any parameters which only have one time step will not get a time dimension
	// This makes sense for such fields as topography

	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 06:00:00"));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2), "2010-03-18 07:00:00"));
	gridData.push_back(TestingGridData(wdb::Parameter("timeless", "stuff"), "2000-01-01 00:00:00"));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	const CDM::DimVec & dims = cdm.getDimensions();

	CDM::DimVec::const_iterator timeDimension = std::find_if(dims.begin(), dims.end(), same_entity("time"));
	BOOST_REQUIRE(timeDimension != dims.end());
	BOOST_CHECK_EQUAL(2, timeDimension->getLength());
}

BOOST_AUTO_TEST_CASE(setsVersionDimensions)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(0));
	gridData.push_back(TestingGridData(1));
	gridData.push_back(TestingGridData(2));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	try
	{
		const CDMDimension & dim = cdm.getDimension("version");
		BOOST_CHECK_EQUAL(3, dim.getLength());
	}
	catch ( CDMException & )
	{
		BOOST_FAIL("Unable to find dimension 'version'");
	}
}

BOOST_AUTO_TEST_CASE(versionsAddThemselvesAsVariables)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(0));
	gridData.push_back(TestingGridData(1));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	const std::string variable = "version";
	try
	{
		cdm.getVariable(variable); // will throw if variable does no exist
		BOOST_CHECK_EQUAL("data version", cdm.getAttribute(variable, "long_name").getStringValue());
		BOOST_CHECK_EQUAL("version", cdm.getAttribute(variable, "standard_name").getStringValue());
	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(zDimensionsAddThemselvesAsVariables)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 1, 1)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 2, 2)));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	const std::string variable = "lvl";
	try
	{
		cdm.getVariable(variable); // will throw if variable does no exist
		BOOST_CHECK_EQUAL("m", cdm.getAttribute(variable, "units").getStringValue());
		BOOST_CHECK_EQUAL("lvl", cdm.getAttribute(variable, "long_name").getStringValue());
		BOOST_CHECK_EQUAL("lvl", cdm.getAttribute(variable, "standard_name").getStringValue());
		BOOST_CHECK_EQUAL("z", cdm.getAttribute(variable, "axis").getStringValue());

	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(manyZDimensions)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 1, 1)));
	gridData.push_back(TestingGridData(wdb::Level("foo bar", "x", 0, 0)));
	gridData.push_back(TestingGridData(wdb::Level("foo bar", "x", 1, 1)));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	const std::string variableA = "lvl";
	const std::string variableB = "foo_bar";
	try
	{
		cdm.getVariable(variableA); // will throw if variableA does no exist
		BOOST_CHECK_EQUAL("m", cdm.getAttribute(variableA, "units").getStringValue());
		BOOST_CHECK_EQUAL("lvl", cdm.getAttribute(variableA, "long_name").getStringValue());
		BOOST_CHECK_EQUAL("lvl", cdm.getAttribute(variableA, "standard_name").getStringValue());
		BOOST_CHECK_EQUAL("z", cdm.getAttribute(variableA, "axis").getStringValue());

		cdm.getVariable(variableB); // will throw if variableB does no exist
		BOOST_CHECK_EQUAL("x", cdm.getAttribute(variableB, "units").getStringValue());
		BOOST_CHECK_EQUAL("foo bar", cdm.getAttribute(variableB, "long_name").getStringValue());
		BOOST_CHECK_EQUAL("foo_bar", cdm.getAttribute(variableB, "standard_name").getStringValue());
		BOOST_CHECK_EQUAL("z", cdm.getAttribute(variableB, "axis").getStringValue());

	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(addsTimeVariable)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData("2011-03-21 06:00:00"));
	gridData.push_back(TestingGridData("2011-03-21 12:00:00"));
	gridData.push_back(TestingGridData("2011-03-21 18:00:00"));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	try
	{
		cdm.getVariable("time");
		BOOST_CHECK_EQUAL("seconds since 1970-01-01 00:00:00 +00:00", cdm.getAttribute("time", "units").getStringValue());
		BOOST_CHECK_EQUAL("time", cdm.getAttribute("time", "long_name").getStringValue());
		BOOST_CHECK_EQUAL("time", cdm.getAttribute("time", "standard_name").getStringValue());
	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}


BOOST_AUTO_TEST_CASE(addsTimeToRelevantVariables)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(TestingGridData::defaultLevel, "2010-03-18 06:00:00"));
	gridData.push_back(TestingGridData(TestingGridData::defaultLevel, "2010-03-18 07:00:00"));
	gridData.push_back(TestingGridData(TestingGridData::defaultLevel, "2010-03-18 08:00:00"));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	try
	{
		const CDMVariable & var = cdm.getVariable(TestingGridData::cdmId());
		const std::vector<std::string> & shape = var.getShape();
		BOOST_REQUIRE_LE(3, shape.size());
		BOOST_CHECK_EQUAL("time", shape[0]);
		BOOST_CHECK_EQUAL("longitude", shape[1]);
		BOOST_CHECK_EQUAL("latitude", shape[2]);
		BOOST_CHECK_EQUAL(3, shape.size());
	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(singleLevelInData)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0)));

	const wdb::DataIndex di(gridData, tr);
	CDM cdm;
	di.populate(cdm);

	try
	{
		const CDMVariable & var = cdm.getVariable(TestingGridData::cdmId());
		const std::vector<std::string> & shape = var.getShape();
		BOOST_REQUIRE_LE(2, shape.size());
		BOOST_CHECK_EQUAL("longitude", shape[0]);
		BOOST_CHECK_EQUAL("latitude", shape[1]);
		BOOST_CHECK_EQUAL(2, shape.size());
	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(severalLevelsInData)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 0, 0)));
	gridData.push_back(TestingGridData(wdb::Level("lvl", "m", 1, 1)));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	try
	{
		const CDMVariable & var = cdm.getVariable(TestingGridData::cdmId());
		const std::vector<std::string> & shape = var.getShape();
		BOOST_REQUIRE_LE(3, shape.size());
		BOOST_CHECK_EQUAL("lvl", shape[0]);
		BOOST_CHECK_EQUAL("longitude", shape[1]);
		BOOST_CHECK_EQUAL("latitude", shape[2]);
		BOOST_CHECK_EQUAL(3, shape.size());
	}
	catch ( CDMException & e )
	{
		BOOST_FAIL(e.what());
	}
}

BOOST_AUTO_TEST_CASE(severalDataVersions)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(0));
	gridData.push_back(TestingGridData(1));
	gridData.push_back(TestingGridData(2));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	const CDMVariable & var = cdm.getVariable(TestingGridData::cdmId());

	const std::vector<std::string> & shape = var.getShape();
	BOOST_REQUIRE_LE(3, shape.size());
	BOOST_CHECK_EQUAL("version", shape[0]);
	BOOST_CHECK_EQUAL("longitude", shape[1]);
	BOOST_CHECK_EQUAL("latitude", shape[2]);
	BOOST_CHECK_EQUAL(3, shape.size());
}

BOOST_AUTO_TEST_CASE(onlyOneTimeDimensionInVaraiableShape)
{
	std::vector<wdb::GridData> gridData;
	gridData.push_back(TestingGridData(0, "2011-03-22 06:00:00"));
	gridData.push_back(TestingGridData(1, "2011-03-22 06:00:00"));
	gridData.push_back(TestingGridData(0, "2011-03-22 07:00:00"));
	gridData.push_back(TestingGridData(1, "2011-03-22 07:00:00"));
	gridData.push_back(TestingGridData(0, "2011-03-22 08:00:00"));
	gridData.push_back(TestingGridData(1, "2011-03-22 08:00:00"));

	const wdb::DataIndex di(gridData, tr);

	CDM cdm;
	di.populate(cdm);

	const CDMVariable & var = cdm.getVariable(TestingGridData::cdmId());

	const std::vector<std::string> & shape = var.getShape();
	BOOST_REQUIRE_LE(4, shape.size());
	BOOST_CHECK_EQUAL("time", shape[0]);
	BOOST_CHECK_EQUAL("version", shape[1]);
	BOOST_CHECK_EQUAL("longitude", shape[2]);
	BOOST_CHECK_EQUAL("latitude", shape[3]);
	BOOST_CHECK_EQUAL(4, shape.size());
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CdmNameTranslator)

BOOST_AUTO_TEST_CASE(testSize)
{
    wdb::CdmNameTranslator translator;
    BOOST_CHECK_EQUAL(true, translator.isEmpty());
    BOOST_CHECK_EQUAL(0, translator.size());

    translator.addNamePair("air temperature", "temperature");
    translator.addNamePair("air pressure", "pressure");
    BOOST_CHECK_EQUAL(2, translator.size());

    translator.addNamePair("geopotential height", "height");
    BOOST_CHECK_EQUAL(3, translator.size());

    translator.addNamePair("longitude", "longitutde");
    translator.addNamePair("latitude", "latitude");
    BOOST_CHECK_EQUAL(5, translator.size());
    BOOST_CHECK_EQUAL(false, translator.isEmpty());

    translator.removeWdbName("geopotential height");
    translator.removeCdmName("temperature");
    BOOST_CHECK_EQUAL(3, translator.size());

    translator.addNamePair("x", "projection_x_coordinate");
    translator.addNamePair("y", "projection_y_coordinate");
    BOOST_CHECK_EQUAL(5, translator.size());

    translator.clear();
    BOOST_CHECK_EQUAL(true, translator.isEmpty());
    BOOST_CHECK_EQUAL(0, translator.size());
}

BOOST_AUTO_TEST_CASE(testHasNames)
{
    wdb::CdmNameTranslator translator;
    BOOST_CHECK_EQUAL(true, translator.isEmpty());

    translator.addNamePair("air temperature", "temperature");
    translator.addNamePair("air pressure", "pressure");
    translator.addNamePair("geopotential height", "height");
    translator.addNamePair("longitude", "longitude");
    translator.addNamePair("latitude", "latitude");
    translator.addNamePair("x", "projection_x_coordinate");
    translator.addNamePair("y", "projection_y_coordinate");

    BOOST_CHECK_EQUAL(7, translator.size());

    BOOST_CHECK_EQUAL(true, translator.hasCdmName("temperature"));
    BOOST_CHECK_EQUAL(true, translator.hasWdbName("air temperature"));

    BOOST_CHECK_EQUAL(true, translator.hasCdmName("projection_x_coordinate"));
    BOOST_CHECK_EQUAL(true, translator.hasWdbName("x"));

    translator.removeCdmName("temperature");
    BOOST_CHECK_EQUAL(false, translator.hasCdmName("temperature"));
    BOOST_CHECK_EQUAL(false, translator.hasWdbName("air temperature"));

    translator.removeWdbName("x");
    BOOST_CHECK_EQUAL(false, translator.hasCdmName("projection_x_coordinate"));
    BOOST_CHECK_EQUAL(false, translator.hasWdbName("x"));

    BOOST_CHECK_EQUAL(false, translator.isEmpty());
}

BOOST_AUTO_TEST_CASE(testTranslation)
{
    wdb::CdmNameTranslator translator;
    BOOST_CHECK_EQUAL(true, translator.isEmpty());

    translator.addNamePair("air temperature", "temperature");
    translator.addNamePair("air pressure", "pressure");
    translator.addNamePair("geopotential height", "height");
    translator.addNamePair("longitude", "longitude");
    translator.addNamePair("latitude", "latitude");
    translator.addNamePair("x", "projection_x_coordinate");
    translator.addNamePair("y", "projection_y_coordinate");

    BOOST_CHECK_EQUAL(7, translator.size());

    BOOST_CHECK_EQUAL("longitutde", translator.toCdmName("longitutde"));
    BOOST_CHECK_EQUAL("longitutde", translator.toWdbName("longitutde"));

    BOOST_CHECK_EQUAL("temperature", translator.toCdmName("air temperature"));
    BOOST_CHECK_EQUAL("geopotential height", translator.toWdbName("height"));

    BOOST_CHECK_EQUAL("not_in_translator", translator.toCdmName("not in translator"));
    BOOST_CHECK_EQUAL("not in translator", translator.toWdbName("not_in_translator"));

    BOOST_CHECK_EQUAL("notintranslator", translator.toCdmName("notintranslator"));
    BOOST_CHECK_EQUAL("notintranslator", translator.toWdbName("notintranslator"));

    BOOST_CHECK_EQUAL(false, translator.isEmpty());
}

BOOST_AUTO_TEST_SUITE_END()

#else
// no boost testframework
int main(int argc, char* args[]) {
}
#endif