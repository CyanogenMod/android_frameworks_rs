/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

struct DetailedFunctionEntry {
    VersionInfo info;
    string htmlDeclaration;
};

static void writeHtmlHeader(GeneratedFile* file) {
    *file << "<!DOCTYPE html>\n";
    *file << "<!-- " << AUTO_GENERATED_WARNING << "-->\n";

    *file << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>\n"
             "<meta name='viewport' content='width=device-width'>\n"
             "<link rel='shortcut icon' type='image/x-icon' "
             "href='http://developer.android.com/favicon.ico'>\n"
             "<title>android.renderscript | Android Developers</title>\n"
             "<!-- STYLESHEETS -->\n"
             "<link rel='stylesheet' "
             "href='http://fonts.googleapis.com/css?family=Roboto+Condensed'>\n"
             "<link rel='stylesheet' href='http://fonts.googleapis.com/"
             "css?family=Roboto:light,regular,medium,thin,italic,mediumitalic,bold' "
             "title='roboto'>\n"
             "<link href='./test_files/default.css' rel='stylesheet' type='text/css'>\n"
             "<!-- FULLSCREEN STYLESHEET -->\n"
             "<link href='./test_files/fullscreen.css' rel='stylesheet' class='fullscreen' "
             "type='text/css'>\n"
             "<!-- JAVASCRIPT -->\n"
             "<script src='./test_files/cb=gapi.loaded_0' async=''></script><script "
             "type='text/javascript' async='' src='./test_files/plusone.js' "
             "gapi_processed='true'></script><script async='' "
             "src='./test_files/analytics.js'></script><script src='./test_files/jsapi' "
             "type='text/javascript'></script>\n"
             "<script src='./test_files/android_3p-bundle.js' type='text/javascript'></script>\n"
             "<script type='text/javascript'>\n"
             "  var toRoot = '/';\n"
             "  var metaTags = [];\n"
             "  var devsite = false;\n"
             "</script>\n"
             "<script src='./test_files/docs.js' type='text/javascript'></script><script "
             "type='text/javascript' src='./test_files/saved_resource'></script>\n"
             "<script>\n"
             "  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){\n"
             "  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),\n"
             "  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)\n"
             "  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');\n"
             "  ga('create', 'UA-5831155-1', 'android.com');\n"
             "  ga('create', 'UA-49880327-2', 'android.com', {'name': 'universal'});  // New "
             "tracker);\n"
             "  ga('send', 'pageview');\n"
             "  ga('universal.send', 'pageview'); // Send page view for new tracker.\n"
             "</script>\n"
             "<link type='text/css' href='./test_files/default+en.css' rel='stylesheet'><script "
             "type='text/javascript' src='./test_files/default+en.I.js'></script></head>\n"
             "<body class='gc-documentation\n"
             "  develop reference'>\n";
    //"  <div id='doc-api-level' class='11' style='display:none'></div>\n"
    //"  <a name='top'></a>\n";
}

static void writeHtmlFooter(GeneratedFile* file) {
    *file << "</div> <!-- end body-content -->\n"
             "</body></html>\n";
}

// If prefix starts input, copy it to stream and remove it from input.
static void skipPrefix(ostringstream* stream, string* input, const string& prefix) {
    size_t size = prefix.size();
    if (input->compare(0, size, prefix) != 0) {
        return;
    }
    input->erase(0, size);
    *stream << prefix;
}

// Merge b into a.  Returns true if successful
static bool mergeVersionInfo(VersionInfo* a, const VersionInfo& b) {
    if (a->intSize != b.intSize) {
        cerr << "Error.  We don't currently support versions that differ based on int size\n";
        return false;
    }
    if (b.minVersion != 0 && a->maxVersion == b.minVersion - 1) {
        a->maxVersion = b.maxVersion;
    } else if (b.maxVersion != 0 && a->minVersion == b.maxVersion + 1) {
        a->minVersion = b.minVersion;
    } else {
        cerr << "Error.  This code currently assume that all versions are contiguous.  Don't know "
                "how to merge versions (" << a->minVersion << " - " << a->maxVersion << ") and ("
             << b.minVersion << " - " << b.maxVersion << ")\n";
        return false;
    }
    return true;
}

static string getHtmlStringForType(const ParameterDefinition& parameter) {
    string s = parameter.rsType;
    ostringstream stream;
    skipPrefix(&stream, &s, "const ");
    skipPrefix(&stream, &s, "volatile ");
    bool endsWithAsterisk = s.size() > 0 && s[s.size() - 1] == '*';
    if (endsWithAsterisk) {
        s.erase(s.size() - 1, 1);
    }

    string anchor = systemSpecification.getHtmlAnchor(s);
    if (anchor.empty()) {
        // Not a RenderScript specific type.
        return parameter.rsType;
    } else {
        stream << anchor;
    }
    if (endsWithAsterisk) {
        stream << "*";
    }
    return stream.str();
}

static string getDetailedHtmlDeclaration(const FunctionPermutation& permutation) {
    ostringstream stream;
    auto ret = permutation.getReturn();
    if (ret) {
        stream << getHtmlStringForType(*ret);
    } else {
        stream << "void";
    }
    stream << " " << permutation.getName() << "(";
    bool needComma = false;
    for (auto p : permutation.getParams()) {
        if (needComma) {
            stream << ", ";
        }
        stream << getHtmlStringForType(*p);
        if (p->isOutParameter) {
            stream << "*";
        }
        if (!p->specName.empty()) {
            stream << " " << p->specName;
        }
        needComma = true;
    }
    stream << ");\n";
    return stream.str();
}

/* Some functions (like max) have changed implementations but not their
 * declaration.  We need to unify these so that we don't end up with entries
 * like:
 *   char max(char a, char b);  Removed from API level 20
 *   char max(char a, char b);  Added to API level 20
 */
static bool getUnifiedFunctionPrototypes(Function* function,
                                         map<string, DetailedFunctionEntry>* entries) {
    for (auto f : function->getSpecifications()) {
        DetailedFunctionEntry entry;
        entry.info = f->getVersionInfo();
        for (auto p : f->getPermutations()) {
            entry.htmlDeclaration = getDetailedHtmlDeclaration(*p);
            const string s = stripHtml(entry.htmlDeclaration);
            auto i = entries->find(s);
            if (i == entries->end()) {
                entries->insert(pair<string, DetailedFunctionEntry>(s, entry));
            } else {
                if (!mergeVersionInfo(&i->second.info, entry.info)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Convert words starting with @ into HTML references.  Returns false if error.
static bool convertDocumentationRefences(string* s) {
    bool success = true;
    size_t end = 0;
    for (;;) {
        size_t start = s->find('@', end);
        if (start == string::npos) {
            break;
        }
        // Find the end of the identifier
        end = start;
        char c;
        do {
            c = (*s)[++end];
        } while (isalnum(c) || c == '_');

        const string id = s->substr(start + 1, end - start - 1);
        string anchor = systemSpecification.getHtmlAnchor(id);
        if (anchor.empty()) {
            cerr << "Error:  Can't convert the documentation reference @" << id << "\n";
            success = false;
        }
        s->replace(start, end - start, anchor);
    }
    return success;
}

static bool generateHtmlParagraphs(GeneratedFile* file, const vector<string>& description) {
    bool inParagraph = false;
    for (auto s : description) {
        // Empty lines in the .spec marks paragraphs.
        if (s.empty()) {
            if (inParagraph) {
                *file << "</p>\n";
                inParagraph = false;
            }
        } else {
            if (!inParagraph) {
                *file << "<p> ";
                inParagraph = true;
            }
        }
        if (!convertDocumentationRefences(&s)) {
            return false;
        }
        *file << s << "\n";
    }
    if (inParagraph) {
        *file << "</p>\n";
    }
    return true;
}

static void writeSummaryTableStart(GeneratedFile* file, const char* label, bool labelIsHeading) {
    if (labelIsHeading) {
        *file << "<h2 style='margin-bottom: 0px;'>" << label << "</h2><hr>\n";
    }
    //#TODO promethods was the id.  implication?
    *file << "<table id='id" << label << "' class='jd-sumtable'><tbody>\n";
    if (!labelIsHeading) {
        *file << "  <tr><th colspan='12'>" << label << "</th></tr>\n";
    }
}

static void writeSummaryTableEnd(GeneratedFile* file) {
    *file << "</tbody></table>\n";
}

static void writeSummaryTableEntry(GeneratedFile* file, Constant* constant) {
    if (constant->hidden()) {
        return;
    }
    *file << "  <tr class='alt-color api apilevel-1'>\n";
    *file << "    <td class='jd-linkcol'><nobr>\n";
    *file << "      <a href='" << constant->getUrl() << "'>" << constant->getName()
          << "</a></nobr>\n";
    *file << "    </td>\n";
    *file << "    <td class='jd-descrcol' width='100%'><nobr>\n";
    *file << "        " << constant->getSummary() << "\n";
    *file << "    </td>\n";
    *file << "  </tr>\n";
}

static void writeSummaryTableEntry(GeneratedFile* file, Type* type) {
    if (type->hidden()) {
        return;
    }
    *file << "  <tr class='alt-color api apilevel-1'>\n";
    *file << "    <td class='jd-linkcol'><nobr>\n";
    *file << "      <a href='" << type->getUrl() << "'>" << type->getName() << "</a></nobr>\n";
    *file << "    </td>\n";
    *file << "    <td class='jd-descrcol' width='100%'><nobr>\n";
    *file << "        " << type->getSummary() << "\n";
    *file << "    </td>\n";
    *file << "  </tr>\n";
}

static void writeSummaryTableEntry(GeneratedFile* file, Function* function) {
    *file << "  <tr class='alt-color api apilevel-1'>\n";
    *file << "    <td class='jd-linkcol'>\n";
    *file << "      <a href='" << function->getUrl() << "'>" << function->getName() << "</a>\n";
    *file << "    </td>\n";
    *file << "    <td class='jd-linkcol' width='100%'>\n";  // TODO jd-typecol
    //    *file << "      <nobr><span class='sympad'></span></nobr>\n";
    *file << "      <div class='jd-descrdiv'>\n";
    *file << "        " << function->getSummary() << "\n";
    *file << "      </div>\n";
    *file << "    </td>\n";
    *file << "  </tr>\n";
}

static void writeSummaryTables(GeneratedFile* file, const map<string, Constant*>& constants,
                               const map<string, Type*>& types,
                               const map<string, Function*>& functions, bool labelAsHeader) {
    if (constants.size() > 0) {
        writeSummaryTableStart(file, "Constants", labelAsHeader);
        for (auto e : constants) {
            writeSummaryTableEntry(file, e.second);
        }
        writeSummaryTableEnd(file);
    }

    if (types.size() > 0) {
        writeSummaryTableStart(file, "Types", labelAsHeader);
        for (auto e : types) {
            writeSummaryTableEntry(file, e.second);
        }
        writeSummaryTableEnd(file);
    }

    if (functions.size() > 0) {
        writeSummaryTableStart(file, "Functions", labelAsHeader);
        for (auto e : functions) {
            writeSummaryTableEntry(file, e.second);
        }
        writeSummaryTableEnd(file);
    }
}

static void writeHtmlVersionTag(GeneratedFile* file, VersionInfo info) {
    if (info.intSize == 32) {
        *file << "For 32 bits: ";
    } else if (info.intSize == 64) {
        *file << "For 64 bits: ";
    }

    if (info.minVersion > 1 || info.maxVersion) {
        *file << "<div>";
        const char* mid =
                    "<a "
                    "href='http://developer.android.com/guide/topics/manifest/"
                    "uses-sdk-element.html#ApiLevels'>API level ";
        if (info.minVersion <= 1) {
            // No minimum
            if (info.maxVersion > 0) {
                *file << "Removed from " << mid << info.maxVersion + 1;
            }
        } else {
            if (info.maxVersion == 0) {
                // No maximum
                *file << "Added in " << mid << info.minVersion;
            } else {
                *file << mid << info.minVersion << " - " << info.maxVersion;
            }
        }
        *file << "</a></div>\n";
    }
}

static void writeDetailedType(GeneratedFile* file, const TypeSpecification* type) {
    switch (type->getKind()) {
        case SIMPLE:
            *file << "Base type: " << type->getSimpleType() << "\n";
            break;
        case ENUM: {
            *file << "An enum<br>\n";
            *file << "    <table class='jd-tagtable'><tbody>\n";

            const vector<string>& values = type->getValues();
            const vector<string>& valueComments = type->getValueComments();
            for (size_t i = 0; i < values.size(); i++) {
                *file << "    <tr><th>" << values[i] << "</th>";
                if (valueComments.size() > i && !valueComments[i].empty()) {
                    *file << "<td>" << valueComments[i] << "</td>";
                }
                *file << "</tr>\n";
            }
            *file << "    </tbody></table>\n";
            break;
        }
        case STRUCT: {
            // TODO string mStructName;             // The name found after the struct keyword
            *file << "A structure<br>\n";
            *file << "    <table class='jd-tagtable'><tbody>\n";
            const vector<string>& fields = type->getFields();
            const vector<string>& fieldComments = type->getFieldComments();
            for (size_t i = 0; i < fields.size(); i++) {
                *file << "    <tr><th>" << fields[i] << "</th>";
                if (fieldComments.size() > i && !fieldComments[i].empty()) {
                    *file << "<td>" << fieldComments[i] << "</td>";
                }
                *file << "</tr>\n";
            }
            *file << "    </tbody></table>\n";
            break;
        }
    }
    writeHtmlVersionTag(file, type->getVersionInfo());
}

static void writeDetailedConstant(GeneratedFile* file, ConstantSpecification* c) {
    *file << "Value: " << c->getValue() << "\n";
    writeHtmlVersionTag(file, c->getVersionInfo());
}

static bool writeOverviewForFile(GeneratedFile* file, const SpecFile& specFile) {
    bool success = true;
    *file << "<h2>" << specFile.getBriefDescription() << "</h2>\n";
    if (!generateHtmlParagraphs(file, specFile.getFullDescription())) {
        success = false;
    }

    // Write the summary tables.
    // file << "<h2>Summary</h2>\n";
    const auto& constants = specFile.getConstantsMap();
    const auto& types = specFile.getTypesMap();
    const auto& functions = specFile.getFunctionsMap();
    writeSummaryTables(file, constants, types, functions, false);
    return success;
}

static bool generateOverview() {
    GeneratedFile file;
    if (!file.start("index.html")) {
        return false;
    }
    bool success = true;
    writeHtmlHeader(&file);

    file << "<h1 itemprop='name'>Overview</h1>\n";
    // TODO Have the overview text here!

    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeOverviewForFile(&file, *specFile)) {
            success = false;
        }
    }

    writeHtmlFooter(&file);
    file.close();
    return success;
}

static bool generateAlphabeticalIndex() {
    GeneratedFile file;
    if (!file.start("alpha_index.html")) {
        return false;
    }
    writeHtmlHeader(&file);

    writeSummaryTables(&file, systemSpecification.getConstants(), systemSpecification.getTypes(),
                       systemSpecification.getFunctions(), true);

    writeHtmlFooter(&file);
    file.close();
    return true;
}

static bool writeDetailedConstant(GeneratedFile* file, Constant* constant) {
    if (constant->hidden()) {
        return true;
    }
    const string& name = constant->getName();

    // TODO need names that distinguish fn.const. type
    // TODO had attr_android:...
    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << constant->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    *file << "    <table class='jd-tagtable'><tbody>\n";
    for (auto f : constant->getSpecifications()) {
        *file << "      <tr><td>";
        writeDetailedConstant(file, f);
        *file << "      </td></tr>\n";
        *file << "<br/>\n";
    }
    *file << "    </tbody></table>\n";
    *file << "  </div>\n";

    *file << "    <div class='jd-tagdata jd-tagdescr'>\n";

    if (!generateHtmlParagraphs(file, constant->getDescription())) {
        return false;
    }
    *file << "    </div>\n";

    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedType(GeneratedFile* file, Type* type) {
    if (type->hidden()) {
        return true;
    }
    const string& name = type->getName();

    // TODO need names that distinguish fn.const. type
    // TODO had attr_android:...
    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << type->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    *file << "    <h5 class='jd-tagtitle'>Variants</h5>\n";
    *file << "    <table class='jd-tagtable'><tbody>\n";
    for (auto f : type->getSpecifications()) {
        *file << "      <tr><td>";
        writeDetailedType(file, f);
        *file << "      </td></tr>\n";
        *file << "<br/>\n";
    }
    *file << "    </tbody></table>\n";
    *file << "  </div>\n";

    *file << "    <div class='jd-tagdata jd-tagdescr'>\n";

    if (!generateHtmlParagraphs(file, type->getDescription())) {
        return false;
    }

    *file << "    </div>\n";

    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedFunction(GeneratedFile* file, Function* function) {
    const string& name = function->getName();

    // TODO need names that distinguish fn.const. type
    // TODO had attr_android:...
    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << function->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    *file << "    <table class='jd-tagtable'><tbody>\n";
    map<string, DetailedFunctionEntry> entries;
    if (!getUnifiedFunctionPrototypes(function, &entries)) {
        return false;
    }
    for (auto i : entries) {
        *file << "      <tr>\n";
        *file << "        <td>" << i.second.htmlDeclaration << "<td/>\n";
        *file << "        <td>";
        writeHtmlVersionTag(file, i.second.info);
        *file << "</td>\n";
        *file << "      </tr>\n";
    }
    *file << "    </tbody></table>\n";
    *file << "  </div>\n";

    if (function->someParametersAreDocumented()) {
        *file << "  <div class='jd-tagdata'>";
        *file << "    <h5 class='jd-tagtitle'>Parameters</h5>\n";
        *file << "    <table class='jd-tagtable'><tbody>\n";
        for (ParameterEntry* p : function->getParameters()) {
            *file << "    <tr><th>" << p->name << "</th><td>" << p->documentation << "</td></tr>\n";
        }
        *file << "    </tbody></table>\n";
        *file << "  </div>\n";
    }

    string ret = function->getReturnDocumentation();
    if (!ret.empty()) {
        *file << "  <div class='jd-tagdata'>";
        *file << "    <h5 class='jd-tagtitle'>Returns</h5>\n";
        *file << "    <table class='jd-tagtable'><tbody>\n";
        *file << "    <tr><td>" << ret << "</td></tr>\n";
        *file << "    </tbody></table>\n";
        *file << "  </div>\n";
    }

    *file << "  <div class='jd-tagdata jd-tagdescr'>\n";
    if (!generateHtmlParagraphs(file, function->getDescription())) {
        return false;
    }
    *file << "  </div>\n";

    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedDocumentationFile(const SpecFile& specFile) {
    GeneratedFile file;
    const string htmlFileName = stringReplace(specFile.getSpecFileName(), ".spec", ".html");
    if (!file.start(htmlFileName)) {
        return false;
    }
    bool success = true;

    writeHtmlHeader(&file);
    file << "<br/>";

    // Write the file documentation.
    file << "<h1 itemprop='name'>" << specFile.getBriefDescription()
         << "</h1>\n";  // TODO not sure about itemprop

    file << "<h2>Overview</h2>\n";
    if (!generateHtmlParagraphs(&file, specFile.getFullDescription())) {
        success = false;
    }

    // Write the summary tables.
    file << "<h2>Summary</h2>\n";
    const auto& constants = specFile.getConstantsMap();
    const auto& types = specFile.getTypesMap();
    const auto& functions = specFile.getFunctionsMap();
    writeSummaryTables(&file, constants, types, functions, false);

    // Write the full details of each constant, type, and function.
    if (!constants.empty()) {
        file << "<h2>Constants</h2>\n";
        for (auto i : constants) {
            if (!writeDetailedConstant(&file, i.second)) {
                success = false;
            }
        }
    }
    if (!types.empty()) {
        file << "<h2>Types</h2>\n";
        for (auto i : types) {
            if (!writeDetailedType(&file, i.second)) {
                success = false;
            }
        }
    }
    if (!functions.empty()) {
        file << "<h2>Functions</h2>\n";
        for (auto i : functions) {
            if (!writeDetailedFunction(&file, i.second)) {
                success = false;
            }
        }
    }

    writeHtmlFooter(&file);
    file.close();

    if (!success) {
        // If in error, write a final message to make it easier to figure out which file failed.
        cerr << htmlFileName << ": Failed due to errors.\n";
    }
    return success;
}

bool generateHtmlDocumentation() {
    bool success = generateOverview() && generateAlphabeticalIndex();
    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeDetailedDocumentationFile(*specFile)) {
            success = false;
        }
    }
    return success;
}
