/*
    Copyright (c) 2002-2009 Tampere University of Technology.

    This file is part of TTA-Based Codesign Environment (TCE).

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
 */
/**
 * @file GenerateBits.cc
 *
 * Implementation of the main function of generatebits application.
 *
 * @author Lasse Laasonen 2005 (lasse.laasonen-no.spam-tut.fi)
 * @author Otto Esko 2009 (otto.esko-no.spam-tut.fi)
 * @note rating: red
 */

#include <iostream>

#include "PIGCmdLineOptions.hh"
#include "ProgramImageGenerator.hh"
#include "PIGCLITextGenerator.hh"

#include "ADFSerializer.hh"
#include "Machine.hh"
#include "ControlUnit.hh"

#include "BinaryEncoding.hh"
#include "BEMSerializer.hh"
#include "BEMGenerator.hh"

#include "Binary.hh"
#include "BinaryStream.hh"
#include "BinaryReader.hh"

#include "FileSystem.hh"

using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

using namespace TPEF;
using namespace TTAMachine;

int const DEFAULT_IMEMWIDTH_IN_MAUS = 1;
int const PERIOD = 10;
string const IMEM_MAU_PKG = "imem_mau_pkg.vhdl";
string const DECOMPRESSOR_FILE = "decompressor.vhdl";

/**
 * Loads the given TPEF file and creates a Binary instance from it.
 *
 * @param tpefFile The TPEF file.
 * @return The newly created Binary instance.
 * @exception InstanceNotFound If instance for reading wasn't found.
 * @exception UnreachableStream If given file can't be read.
 * @exception KeyAlreadyExists Key was in use when trying to register object.
 * @exception EndOfFile If end of file were reached while it shouldn't.
 * @exception OutOfRange Some read value was out of range.
 * @exception WrongSubclass Some class couldn't do what it was asked for.
 * @exception UnexpectedValue If there was unexpected value when reading.
 */
static Binary*
loadTPEF(const std::string& tpefFile)
    throw (InstanceNotFound, UnreachableStream, KeyAlreadyExists, EndOfFile,
           OutOfRange, WrongSubclass, UnexpectedValue) {

    BinaryStream stream(tpefFile);
    return BinaryReader::readBinary(stream);
}


/**
 * Loads the given BEM file and creates a BinaryEncoding instance from it.
 *
 * @param bemFile The BEM file.
 * @return The newly created BinaryEncoding instance.
 * @exception SerializerException If an error occurs while reading the 
 *                                file.
 * @exception ObjectStateLoadingException If an error occurs while loading
 *                                        the state of BinaryEncoding 
 *                                        instance.
 */
static BinaryEncoding*
loadBEM(const std::string& bemFile) 
    throw (SerializerException, ObjectStateLoadingException) {

    BEMSerializer serializer;
    serializer.setSourceFile(bemFile);
    return serializer.readBinaryEncoding();
}


/**
 * Loads the given ADF file and creates a Machine instance from it.
 *
 * @param adfFile The ADF file.
 * @return The newly created Machine instance.
 * @exception SerializerException If an error occurs while reading the file.
 * @exception ObjectStateLoadingException If an error occurs while loading
 *                                        the state of Machine instance.
 */
static Machine*
loadMachine(const std::string& adfFile) 
    throw (SerializerException, ObjectStateLoadingException) {

    ADFSerializer serializer;
    serializer.setSourceFile(adfFile);
    return serializer.readMachine();
}


/**
 * Tells whether the given address space is instruction memory.
 *
 * @param as The address space.
 * @return True if the address space is instruction memory, otherwise false.
 */
static bool
isInstructionMemory(const TTAMachine::AddressSpace& as) {
    Machine* mach = as.machine();
    ControlUnit* gcu = mach->controlUnit();
    if (gcu != NULL && gcu->addressSpace() == &as) {
        return true;
    } else {
        return false;
    }
}


/**
 * Returns the name of the program's imem image file for the given TPEF file.
 *
 * @param tpefFile Name of the TPEF file.
 * @param format The image output format
 * @return Name of the program image file.
 */
std::string
programImemImageFile(const std::string& tpefFile, const std::string& format) {
    string imageFile = FileSystem::fileNameBody(tpefFile);
    if (format == "mif") {
        // altera tools want .mif ending for MIF files
        imageFile += ".mif";
    } else {
        imageFile += ".img";
    }  
    return imageFile;
}

/**
 * Returns the name of the program data image file for the given TPEF file.
 *
 * @param tpefFile Name of the TPEF file.
 * @param format The image output format
 * @param asName Name of the address space the image belongs to.
 * @return Name of the program image file.
 */
std::string
programDataImageFile(
    const std::string& tpefFile,
    const std::string& format,
    const std::string& asName) {
    
    string imageFile = FileSystem::fileNameBody(tpefFile) + "_" + asName;
    if (format == "mif") {
        imageFile += ".mif";
    } else {
        imageFile += ".img";
    }
    return imageFile;
}

/**
 * Parses the given parameter which has form 'paramname=paramvalue" to
 * different strings.
 *
 * @param param The parameter.
 * @param paramName Parameter name is stored here.
 * @param paramValue Parameter value is stored here.
 * @exception InvalidData If the given parameter is not in the correct form.
 */
void
parseParameter(
    const std::string& param,
    std::string& paramName,
    std::string& paramValue)
    throw (InvalidData) {

    string::size_type separatorPos = param.find("=", 0);
    if (separatorPos == string::npos) {
        string errorMsg = 
            "Compressor parameters must be in form "
            "'parametername=parametervalue'.";
        throw InvalidData(__FILE__, __LINE__, __func__, errorMsg);
    }

    paramName = param.substr(0, separatorPos);
    paramValue = param.substr(separatorPos+1, param.length());
}


void 
createMauPkg(int imemMauWidth, string fileName) {
    string indentation = "   ";
 
    if (!FileSystem::fileExists(fileName) 
        && !FileSystem::createFile(fileName)) {
        string errorMsg = "Unable to create file " + fileName;
        throw IOException(__FILE__, __LINE__, __func__, errorMsg);
    } else if (!FileSystem::fileIsWritable(fileName)) {
        string errorMsg = "Unable to write to file " + fileName;
        throw IOException(__FILE__, __LINE__, __func__, errorMsg);
    }
    std::ofstream stream(fileName.c_str());

    stream << "package imem_mau is" << endl

           << indentation << "-- created by generatebits" << endl
           << indentation << "constant IMEMMAUWIDTH : positive := "
           << imemMauWidth << ";" << endl
           << "end imem_mau;" << endl;
    stream.close();
}

void
createCompressor(string fileName, ProgramImageGenerator& imageGenerator) {
    bool created = FileSystem::createFile(fileName);
    if (!created) {
        string errorMsg = "Unable to create file " + 
            fileName;
        throw IOException(__FILE__, __LINE__, __func__, errorMsg);
    }
    std::ofstream decompressorStream(
        fileName.c_str(), std::ofstream::out);
    imageGenerator.generateDecompressor(decompressorStream);
    decompressorStream.close();
}

/**
 * The main function of generatebits application.
 */
int main(int argc, char* argv[]) {

    PIGCmdLineOptions options;
    try {
        options.parse(argv, argc);
    } catch (ParserStopRequest) {
        return EXIT_SUCCESS;
    } catch (const IllegalCommandLine& exception) {
        cerr << exception.errorMessage() << endl;
        return EXIT_FAILURE;
    }
    
    string adfFile = "";
    if (options.numberOfArguments() < 1) {
        PIGCLITextGenerator textGen;
        cerr << textGen.text(PIGCLITextGenerator::TXT_ADF_REQUIRED) << endl;
        return EXIT_FAILURE;
    }
    else if (options.numberOfArguments() > 1) {
        PIGCLITextGenerator textGen;
        cerr << textGen.text(PIGCLITextGenerator::TXT_ILLEGAL_ARGS) << endl;
        return EXIT_FAILURE;
    }
    else {
        adfFile = options.argument(1);
    }
    
    string bemFile = options.bemFile();
    string piFormat = options.programImageOutputFormat();
    string diFormat = options.dataImageOutputFormat();
    int dmemMAUsPerLine = options.dataMemoryWidthInMAUs();
    string compressor = options.compressorPlugin();
    bool generateDataImages = options.generateDataImages();
    bool generateDecompressor = options.generateDecompressor();
    bool showCompressors = options.showCompressors();
    int imemMAUsPerLine = DEFAULT_IMEMWIDTH_IN_MAUS;
    string progeOutputDir = options.progeOutputDirectory();

    if (showCompressors) {
        std::vector<string> compressorFiles = 
            ProgramImageGenerator::availableCompressors();
        for (std::vector<string>::const_iterator iter = 
                 compressorFiles.begin();
             iter != compressorFiles.end(); iter++) {
            try {
                std::cout << "******************************************"
                          << "**********************" << endl;
                std::cout << "Compressor file: " << *iter << endl;
             
                ProgramImageGenerator::printCompressorDescription(
                    *iter, std::cout);
            } catch (const Exception& e) {
                cerr << "Error: " << e.errorMessage() << endl;
            }
            std::cout << std::endl;
        }
        return EXIT_SUCCESS;
    }       

    CodeCompressorPlugin::ParameterTable compressorParams;
    for (int i = 0; i < options.compressorParameterCount(); i++) {
        string param = options.compressorParameter(i);
        string paramName;
        string paramValue;
        try {
            parseParameter(param, paramName, paramValue);
        } catch (const Exception& e) {
            cerr << e.errorMessage() << endl;
            return EXIT_FAILURE;
        }
        CodeCompressorPlugin::Parameter newParam = {paramName, paramValue};
        compressorParams.push_back(newParam);
    }
    
    if (adfFile == "" || 
        (piFormat != "" && piFormat != "binary" && piFormat != "ascii" &&
         piFormat != "array" && piFormat != "mif") || 
        (diFormat != "" && diFormat != "binary" &&
         diFormat != "ascii" && diFormat != "array" && diFormat != "mif")) {
        options.printHelp();
        return EXIT_FAILURE;
    }

    std::vector<Binary*> tpefTable;        
    ProgramImageGenerator::TPEFMap tpefMap;
    try {
        Machine* mach = loadMachine(adfFile);

        for (int i = 0; i < options.tpefFileCount(); i++) {
            string tpefFile = options.tpefFile(i);
            Binary* tpef = loadTPEF(tpefFile);
            tpefTable.push_back(tpef);
            tpefMap[FileSystem::fileOfPath(tpefFile)] = tpef;
        }

        BinaryEncoding* bem = NULL;
        
        if (bemFile != "") {
            bem = loadBEM(bemFile);
        } else {
            PIGCLITextGenerator textGen;
            std::cerr 
                << textGen.text(PIGCLITextGenerator::TXT_GENERATING_BEM).
                str() << std::endl;
            BEMGenerator bemGenerator(*mach);
            bem = bemGenerator.generate();
        }

        ProgramImageGenerator imageGenerator;
        if (compressor != "") {
            imageGenerator.loadCompressorPlugin(compressor);
        }
        imageGenerator.loadCompressorParameters(compressorParams);
        imageGenerator.loadBEM(*bem);
        imageGenerator.loadMachine(*mach);        
        imageGenerator.loadPrograms(tpefMap);

        for (size_t i = 0; i < tpefTable.size(); i++) {

            Binary* program = tpefTable[i];
            string tpefFile = FileSystem::fileOfPath(options.tpefFile(i));
            string imageFile = programImemImageFile(tpefFile, piFormat);
            ofstream piStream(imageFile.c_str());
            if (piFormat == "binary") {
                imageGenerator.generateProgramImage(
                    tpefFile, piStream, ProgramImageGenerator::BINARY);
            } else if (piFormat == "array") {
                imageGenerator.generateProgramImage(
                    tpefFile, piStream, ProgramImageGenerator::ARRAY,
                    imemMAUsPerLine);
            } else if (piFormat == "mif") {
                imageGenerator.generateProgramImage(
                    tpefFile, piStream, ProgramImageGenerator::MIF,
                    imemMAUsPerLine);
            } else {
                assert(piFormat == "ascii" || piFormat == "");
                imageGenerator.generateProgramImage(
                    tpefFile, piStream, ProgramImageGenerator::ASCII,
                    imemMAUsPerLine);
            }
            
            piStream.close();

            if (generateDataImages) {
                Machine::AddressSpaceNavigator asNav = 
                    mach->addressSpaceNavigator();
                for (int i = 0; i < asNav.count(); i++) {
                    AddressSpace* as = asNav.item(i);
                    if (!isInstructionMemory(*as)) {
                        string fileName = 
                            programDataImageFile(
                                tpefFile, diFormat, as->name());
                        ofstream stream(fileName.c_str());
                        if (diFormat == "binary") {
                            imageGenerator.generateDataImage(
                                tpefFile, *program, as->name(), stream, 
                                ProgramImageGenerator::BINARY, 1, true);
                        } else if (diFormat == "array") {
                            imageGenerator.generateDataImage(
                                tpefFile, *program, as->name(), stream, 
                                ProgramImageGenerator::ARRAY,
                                dmemMAUsPerLine, true);
                        } else if (diFormat == "mif") {
                            imageGenerator.generateDataImage(
                                tpefFile, *program, as->name(), stream, 
                                ProgramImageGenerator::MIF,
                                dmemMAUsPerLine, true);
                        } else {
                            assert(diFormat == "ascii" || diFormat == "");
                            imageGenerator.generateDataImage(
                                tpefFile, *program, as->name(), stream, 
                                ProgramImageGenerator::ASCII,
                                dmemMAUsPerLine, true);
                        }
                        stream.close();
                    }
                }
            }
        }

        if (generateDecompressor) {
            string decomp = DECOMPRESSOR_FILE;
            if (!progeOutputDir.empty()) {
                string temp = progeOutputDir 
                    + FileSystem::DIRECTORY_SEPARATOR
                    + "gcu_ic" + FileSystem::DIRECTORY_SEPARATOR 
                    + DECOMPRESSOR_FILE;
               if ( (FileSystem::fileExists(temp) 
                  &&FileSystem::fileIsWritable(temp)) 
                 || FileSystem::fileIsCreatable(temp)) {
                   decomp = temp;
               }
            }
            createCompressor(decomp, imageGenerator);            
        }
        
        int compressedInstructionWidth = imageGenerator.imemMauWidth();
        string imemMauPkg = IMEM_MAU_PKG;
        if (!progeOutputDir.empty()) {
             string temp = progeOutputDir 
                + FileSystem::DIRECTORY_SEPARATOR
                + "vhdl" + FileSystem::DIRECTORY_SEPARATOR + IMEM_MAU_PKG;
            if ( (FileSystem::fileExists(temp) 
                  &&FileSystem::fileIsWritable(temp)) 
                 || FileSystem::fileIsCreatable(temp)) {
                imemMauPkg = temp;
            }
        }
        createMauPkg(compressedInstructionWidth, imemMauPkg);

        for (std::vector<Binary*>::iterator iter = tpefTable.begin();
             iter != tpefTable.end(); iter++) {
            delete *iter;
        }
        delete mach;
        delete bem;

        return EXIT_SUCCESS;

    } catch (const Exception& exception) {
        cerr << exception.errorMessageStack() << endl;
        return EXIT_FAILURE;
    }
}
