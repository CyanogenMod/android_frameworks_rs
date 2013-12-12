/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <stdio.h>
#include <vector>
#include <list>
#include <string>

using namespace std;

FILE *gIn;
FILE *gOut;

class Func {
public:
    Func() {
        mMinVersion = 0;
        mMaxVersion = 0;
    }

    string mName;
    int mMinVersion;
    int mMaxVersion;

    vector<vector<string> > mReplaceables;
    vector<string> mArgs;
    string mRet;
    vector<string> mComment;
    vector<string> mInline;

};

vector<Func *> gFuncs;

bool getNextLine(FILE *in, string *s) {
    s->clear();
    while (1) {
        int c = fgetc(in);
        if (c == EOF) return s->size() != 0;
        if (c == '\n') break;
        s->push_back((char)c);
    }
    return true;
}

void trim(string *s, size_t start) {
    if (start > 0) {
        s->erase(0, start);
    }

    while (s->size() && (s->at(0) == ' ')) {
        s->erase(0, 1);
    }

    size_t p = s->find_first_of("\n\r");
    if (p != string::npos) {
        s->erase(p);
    }

    while ((s->size() > 0) && (s->at(s->size()-1) == ' ')) {
        s->erase(s->size() -1);
    }
}

Func * scanFunction(FILE *in) {
    Func *f = new Func();
    bool modeComment = false;
    bool modeInline = false;
    size_t replacables = 0;

    while (1) {
        string s;
        bool ret = getNextLine(in, &s);
        if (!ret) break;

        if (modeComment) {
            if (!s.size() || (s[0] == ' ')) {
                trim(&s, 0);
                f->mComment.push_back(s);
                continue;
            } else {
                modeComment = false;
            }
        }

        if (modeInline) {
            if (!s.size() || (s[0] == ' ')) {
                trim(&s, 0);
                f->mInline.push_back(s);
                continue;
            } else {
                modeInline = false;
            }
        }

        if (s[0] == '#') {
            continue;
        }

        if (s.compare(0, 5, "name:") == 0) {
            trim(&s, 5);
            f->mName = s;
            continue;
        }

        if (s.compare(0, 4, "arg:") == 0) {
            trim(&s, 4);
            f->mArgs.push_back(s);
            continue;
        }

        if (s.compare(0, 4, "ret:") == 0) {
            trim(&s, 4);
            f->mRet = s;
            continue;
        }

        if (s.compare(0, 4, "end:") == 0) {
            return f;
        }

        if (s.compare(0, 8, "comment:") == 0) {
            modeComment = true;
            continue;
        }

        if (s.compare(0, 7, "inline:") == 0) {
            modeInline = true;
            continue;
        }

        if (s.compare(0, 8, "version:") == 0) {
            trim(&s, 8);
            sscanf(s.c_str(), "%i %i", &f->mMinVersion, &f->mMaxVersion);
            continue;
        }

        if (s.compare(0, 8, "start:") == 0) {
            continue;
        }

        if (s.compare(0, 2, "w:") == 0) {
            vector<string> t;
            if (s.find("1") != string::npos) {
                t.push_back("");
            }
            if (s.find("2") != string::npos) {
                t.push_back("2");
            }
            if (s.find("3") != string::npos) {
                t.push_back("3");
            }
            if (s.find("4") != string::npos) {
                t.push_back("4");
            }
            f->mReplaceables.push_back(t);
            continue;
        }

        if (s.compare(0, 2, "t:") == 0) {
            vector<string> t;
            if (s.find("f16") != string::npos) {
                t.push_back("half");
            }
            if (s.find("f32") != string::npos) {
                t.push_back("float");
            }
            if (s.find("f64") != string::npos) {
                t.push_back("double");
            }
            if (s.find("i8") != string::npos) {
                t.push_back("char");
            }
            if (s.find("u8") != string::npos) {
                t.push_back("uchar");
            }
            if (s.find("i16") != string::npos) {
                t.push_back("short");
            }
            if (s.find("u16") != string::npos) {
                t.push_back("ushort");
            }
            if (s.find("i32") != string::npos) {
                t.push_back("int");
            }
            if (s.find("u32") != string::npos) {
                t.push_back("uint");
            }
            if (s.find("i64") != string::npos) {
                t.push_back("long");
            }
            if (s.find("u64") != string::npos) {
                t.push_back("ulong");
            }
            f->mReplaceables.push_back(t);
            continue;
        }

        if (s.size() == 0) {
            // eat empty line
            continue;
        }

        printf("Error, line:\n");
        printf("  %s\n", s.c_str());
    }

    delete f;
    return NULL;
}

string stringReplace(string s, string match, string rep) {
    while(1) {
        size_t p = s.find(match);
        if (p == string::npos) break;

        s.erase(p, match.size());
        s.insert(p, rep);
    }
    return s;
}

string stringExpand(string s, const Func *f, int i1, int i2, int i3, int i4) {
    if (f->mReplaceables.size() > 0) {
        s = stringReplace(s, "#1", f->mReplaceables[0][i1]);
    }
    if (f->mReplaceables.size() > 1) {
        s = stringReplace(s, "#2", f->mReplaceables[1][i2]);
    }
    if (f->mReplaceables.size() > 2) {
        s = stringReplace(s, "#3", f->mReplaceables[2][i3]);
    }
    if (f->mReplaceables.size() > 3) {
        s = stringReplace(s, "#4", f->mReplaceables[3][i4]);
    }
    return s;
}

void writeHeaderFunc(FILE *o, const Func *f, int i1, int i2, int i3, int i4) {
    string s;

    if (f->mMinVersion || f->mMaxVersion) {
        if (f->mMaxVersion) {
            fprintf(o, "#if (defined(RS_VERSION) && (RS_VERSION >= %i) && (RS_VERSION < %i))\n",
                    f->mMinVersion, f->mMaxVersion);
        } else {
            fprintf(o, "#if (defined(RS_VERSION) && (RS_VERSION >= %i))\n",
                    f->mMinVersion);
        }
    }

    fprintf(o, "/*\n");
    for (size_t ct=0; ct < f->mComment.size(); ct++) {
        s = stringExpand(f->mComment[ct], f, i1, i2, i3, i4);
        if (s.size()) {
            fprintf(o, " * %s\n", s.c_str());
        } else {
            fprintf(o, " *\n");
        }
    }
    fprintf(o, " *\n");
    if (f->mMinVersion || f->mMaxVersion) {
        if (f->mMaxVersion) {
            fprintf(o, " * Suppored by API versions %i - %i\n",
                    f->mMinVersion, f->mMaxVersion);
        } else {
            fprintf(o, " * Supported by API versions %i and newer.\n",
                    f->mMinVersion);
        }
    }

    fprintf(o, " */\n");

    s.clear();
    s += "extern ";
    s += f->mRet;
    s += " __attribute__((const, overloadable))";
    s += f->mName;
    s += "(";
    if (f->mArgs.size()) {
        s += f->mArgs[0];
    }
    for (size_t ct=1; ct < f->mArgs.size(); ct++) {
        s += ", ";
        s += f->mArgs[0];
    }
    s += ");";
    s = stringExpand(s, f, i1, i2, i3, i4);
    fprintf(o, "%s\n", s.c_str());


    if (f->mMinVersion || f->mMaxVersion) {
        fprintf(o, "#endif\n");
    }

    fprintf(o, "\n", s.c_str());
}


void writeHeaderFuncs(FILE *o, const Func *f) {
    switch(f->mReplaceables.size()) {
    case 0:
        writeHeaderFunc(o, f, -1, -1, -1, -1);
        break;
    case 1:
        for (size_t i1 = 0; i1 < f->mReplaceables[0].size(); i1++) {
            writeHeaderFunc(o, f, i1, -1, -1, -1);
        }
        break;
    case 2:
        for (size_t i2 = 0; i2 < f->mReplaceables[1].size(); i2++) {
            for (size_t i1 = 0; i1 < f->mReplaceables[0].size(); i1++) {
                writeHeaderFunc(o, f, i1, i2, -1, -1);
            }
        }
        break;
    case 3:
        for (size_t i3 = 0; i3 < f->mReplaceables[2].size(); i3++) {
            for (size_t i2 = 0; i2 < f->mReplaceables[1].size(); i2++) {
                for (size_t i1 = 0; i1 < f->mReplaceables[0].size(); i1++) {
                    writeHeaderFunc(o, f, i1, i2, i3, -1);
                }
            }
        }
        break;
    case 4:
        for (size_t i4 = 0; i4 < f->mReplaceables[3].size(); i4++) {
            for (size_t i3 = 0; i3 < f->mReplaceables[2].size(); i3++) {
                for (size_t i2 = 0; i2 < f->mReplaceables[1].size(); i2++) {
                    for (size_t i1 = 0; i1 < f->mReplaceables[0].size(); i1++) {
                        writeHeaderFunc(o, f, i1, i2, i3, i4);
                    }
                }
            }
        }
        break;
    }
}



int main(int argc, char* argv[])
{
    const char *inpath = "runtime.spec";
    const char *outpath = "rs_core_math.rsh";

    gIn = fopen(inpath, "rt");
    if (!gIn) {
        printf("Error opening input file: %s", inpath);
        return -1;
    }

    while (1) {
        Func *f = scanFunction(gIn);
        if (f != NULL) {
            gFuncs.push_back(f);
        } else {
            break;
        }
    }

    gOut = fopen(outpath, "wt");
    if (!gOut) {
        printf("Error opening output file: %s", outpath);
        return -1;
    }

    for (size_t ct=0; ct < gFuncs.size(); ct++) {
        writeHeaderFuncs(gOut, gFuncs[ct]);
    }

    fclose (gIn);
    fclose (gOut);

    printf("%i Functions processed.\n", (int)gFuncs.size());

    return 0;
}


