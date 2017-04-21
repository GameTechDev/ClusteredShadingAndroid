/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "map"
#include "vector"
#include "CPUT.h"
#include "CPUTParser.h"

//
// The Command Parser class will retrieve values from a string and store them internally.
// An application can then query for different parameters. All of the query functions are
// guaranteed to not modify the return variable if the parameter was not found.
//

CommandParser::~CommandParser()
{
	CleanConfigurationOptions();
}

//
// This function parses configuration options from a text string. Removes any previous
// options stored in the configuration list.
//
void CommandParser::ParseConfigurationOptions(cString arguments, cString delimiter)
{
	CleanConfigurationOptions();

	std::vector<cString> argumentList;

	size_t pos;
	size_t nextPos = arguments.find_first_of(delimiter, 0);

	//
	// Break out parameters from command line
	//
	while (nextPos != std::string::npos) {
		pos = nextPos + 1;
		nextPos = arguments.find_first_of(delimiter, pos);
		argumentList.push_back(arguments.substr(pos, nextPos - pos));
	}

	//
	// Remove leading spaces from arguments.
	//
	for (std::vector<cString>::iterator it = argumentList.begin(); it != argumentList.end(); it++) {
		std::string::size_type pos = it->find_first_not_of(' ');
		if (pos != std::string::npos) {
			it->erase(0, pos);
		}
	}

	//
	// Remove trailing spaces from arguments
	//
	for (std::vector<cString>::iterator it = argumentList.begin(); it != argumentList.end(); it++) {
		std::string::size_type pos = it->find_last_not_of(' ');
		if (pos != std::string::npos) {
			it->erase(pos + 1);
		}
	}

	//
	// Split the values from the parameter name
	//
	cString arg;
	for (std::vector<cString>::iterator it = argumentList.begin(); it != argumentList.end(); it++) {
		arg = *it;
		pos = arg.find_first_of(_L(":"), 0);
		if (pos != cString::npos) {
			m_ArgumentMap.insert(std::make_pair(arg.substr(0, pos), arg.substr(pos + 1, std::string::npos)));
		} else {
			m_ArgumentMap.insert(std::make_pair(arg.substr(0, pos), _L("")));
		}
	}

	return;
}

void CommandParser::CleanConfigurationOptions(void)
{
	m_ArgumentMap.clear();

	return;
}

bool CommandParser::GetParameter(cString arg)
{
	std::map<cString, cString>::iterator it;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

	return true;
}

bool CommandParser::GetParameter(cString arg, int *pOut)
{
	std::map<cString, cString>::iterator it;
	cStringStream ss;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

	ss << it->second;
	ss >> *pOut;

	return true;
}

bool CommandParser::GetParameter(cString arg, double *pOut)
{
	std::map<cString, cString>::iterator it;
	cStringStream ss;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

	ss << it->second;
	ss >> *pOut;

	return true;
}

bool CommandParser::GetParameter(cString arg, unsigned int *pOut)
{
	std::map<cString, cString>::iterator it;
	cStringStream ss;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

	ss << it->second;
	ss >> *pOut;

	return true;
}

bool CommandParser::GetParameter(cString arg, cString *pOut)
{
	std::map<cString, cString>::iterator it;
	cStringStream ss;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

	ss << it->second;
	*pOut = ss.str();

	return true;
}

// buffer pointed to by pOut must be large enough for data
bool CommandParser::GetParameter(cString arg, char *pOut)
{
	std::map<cString, cString>::iterator it;
	std::stringstream ss;

	it = m_ArgumentMap.find(arg);
	if (it == m_ArgumentMap.end())
		return false;

#ifdef 	CPUT_OS_ANDROID
    ss << ws2s(it->second.c_str());
#else
    ss << ws2s(it->second);
#endif
	ss >> pOut;

	return true;
}