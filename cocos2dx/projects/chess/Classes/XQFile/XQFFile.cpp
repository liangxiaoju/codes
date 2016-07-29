#include "XQFFile.h"
#include "Utils.h"
#include "PositionStruct.h"

void XQFFile::load(std::string name)
{
    std::string filename = FileUtils::getInstance()->fullPathForFilename(name);
    _xqfData = FileUtils::getInstance()->getDataFromFile(filename);

    _fileOffset = 0;

    if (!checkFormat())
        return;

    memcpy(&_xqfHeader, _xqfData.getBytes(), sizeof(_xqfHeader));

    initDecryptKey();

    initXQHeader();

    _fileOffset = 1024;
    _data.pRoot = readStep(nullptr);
}

XQNode *XQFFile::readStep(XQNode *parent)
{
    uint8_t ucStep[4];
    uint8_t ucTmp = 0;
    int nCommentLen = 0;
    char *szComment = 0;

    readAndDecrypt(ucStep, 4);

    if (_xqfHeader.Version <= 0xA) {
        if (ucStep[2] & 0xF0)
            ucTmp |= 0x80;
        if (ucStep[2] & 0x0F)
            ucTmp |= 0x40;
        ucStep[2] = ucTmp;
        readAndDecrypt(&nCommentLen, sizeof(int));
    } else {
        ucStep[2] &= 0xE0;
        if (ucStep[2] & 0x20) {
            readAndDecrypt(&nCommentLen, sizeof(int));
            nCommentLen -= _commentOffset;
        }
    }

    ucStep[0] -= 24;
    ucStep[0] -= _srcOffset;
    ucStep[1] -= 32;
    ucStep[1] -= _dstOffset;

    XQNode *node = new XQNode();
    if (node == nullptr)
        return nullptr;

    node->pParent = parent;

    auto s = Vec2(ucStep[0]/10, ucStep[0]%10);
    auto d = Vec2(ucStep[1]/10, ucStep[1]%10);
    node->mv = Utils::toUcciMove(s, d);
    log("mv: %s", node->mv.c_str());

    if (nCommentLen > 0) {
        szComment = new char[nCommentLen + 1];
        memset(szComment, 0, nCommentLen+1);
        readAndDecrypt(szComment, nCommentLen);
        szComment[nCommentLen] = 0;
        node->comment = Utils::convertGBKToUTF8(szComment);
        delete [] szComment;
        log("comment: %s", node->comment.c_str());
    }

    if (ucStep[2] & 0x80) {
        XQNode *child = readStep(node);
        if (child) {
            node->pFirstChild = child;
            node->pCurChild = child;
        }
    }

    if (ucStep[2] & 0x40) {
        XQNode *right = readStep(parent);
        if (right) {
            node->pRight = right;
            right->pLeft = node;
        }
    }

    return node;
}

void XQFFile::readAndDecrypt(void *_buf, int len)
{
    unsigned char *buf = (unsigned char *)_buf;
    unsigned char *bytes = _xqfData.getBytes();
    int offset = _fileOffset;
    _fileOffset += len;

    memcpy(buf, bytes+offset, len);

    for (int i = 0; i < len; i++) {
        buf[i] -= _encKey[(offset+i)%32];
    }
}

void XQFFile::initXQHeader()
{
    _data.header.type = _xqfHeader.CodeA;
    _data.header.title = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.Title, _xqfHeader.TitleLen));
    _data.header.matchName = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.MatchName, _xqfHeader.MatchNameLen));
    _data.header.matchAddr = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.Addr, _xqfHeader.AddrLen));
    _data.header.whiteName = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.RedPlayer, _xqfHeader.RedPlayerLen));
    _data.header.blackName = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.BlackPlayer, _xqfHeader.BlackPlayerLen));
    _data.header.author = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.Author, _xqfHeader.AuthorLen));
    _data.header.annotator = Utils::convertGBKToUTF8(std::string((char*)_xqfHeader.Annotator, _xqfHeader.AnnotatorLen));
    _data.header.fen = genInitFen();
    log("fen: %s", _data.header.fen.c_str());
    log("title: %s", _data.header.title.c_str());
}

bool XQFFile::checkFormat()
{
    unsigned char *bytes = _xqfData.getBytes();

    // magic
    if (bytes[0] != 'X' || bytes[1] != 'Q')
        return false;

    // version
//    if (bytes[2] > 0x12)
//        return false;

    // sum
//    if (std::accumulate(bytes+0xc, bytes+0x10, 0) != 0)
//        return false;

    return true;
}

void XQFFile::initDecryptKey()
{
    unsigned char *bytes = _xqfData.getBytes();

    auto Square54Plus221 = [](int x) {
        return x * x * 54 + 221;
    };

    const char *const encKeyMask = "[(C) Copyright Mr. Dong Shiwei.]";

    if (_xqfHeader.Version < 11) {
        _pieceOffset = 0;
        _srcOffset = 0;
        _dstOffset = 0;
        _commentOffset = 0;
        memset(_encKey, 0, sizeof(_encKey));
    } else {
        _pieceOffset = (uint8_t)(Square54Plus221(bytes[0xd]) * bytes[0xd]);
        _srcOffset = (uint8_t)(Square54Plus221(bytes[0xe]) * _pieceOffset);
        _dstOffset = (uint8_t)(Square54Plus221(bytes[0xf]) * _srcOffset);
        _commentOffset = ((bytes[0xc] * 256 + bytes[0xd]) % 0x7d00) + 0x2ff;

        int arg0 = bytes[3];
        int args[4];
        for (int i = 0; i < 4; i++) {
            args[i] = bytes[8+i] | (bytes[12+i] & arg0);
        }
        for (int i = 0; i < 32; i++) {
            _encKey[i] = (uint8_t)(args[i % 4] & encKeyMask[i]);
        }
    }
    log("pieceOffset: %d", _pieceOffset);
    log("srcOffset: %d", _srcOffset);
    log("dstOffset: %d", _dstOffset);
    log("commentOffset: %d", _commentOffset);
}

static const unsigned char cucsqXqf2Square[96] = {
  0xc3, 0xb3, 0xa3, 0x93, 0x83, 0x73, 0x63, 0x53, 0x43, 0x33,
  0xc4, 0xb4, 0xa4, 0x94, 0x84, 0x74, 0x64, 0x54, 0x44, 0x34,
  0xc5, 0xb5, 0xa5, 0x95, 0x85, 0x75, 0x65, 0x55, 0x45, 0x35,
  0xc6, 0xb6, 0xa6, 0x96, 0x86, 0x76, 0x66, 0x56, 0x46, 0x36,
  0xc7, 0xb7, 0xa7, 0x97, 0x87, 0x77, 0x67, 0x57, 0x47, 0x37,
  0xc8, 0xb8, 0xa8, 0x98, 0x88, 0x78, 0x68, 0x58, 0x48, 0x38,
  0xc9, 0xb9, 0xa9, 0x99, 0x89, 0x79, 0x69, 0x59, 0x49, 0x39,
  0xca, 0xba, 0xaa, 0x9a, 0x8a, 0x7a, 0x6a, 0x5a, 0x4a, 0x3a,
  0xcb, 0xbb, 0xab, 0x9b, 0x8b, 0x7b, 0x6b, 0x5b, 0x4b, 0x3b,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const int cpcXqf2Piece[32] = {
  23, 21, 19, 17, 16, 18, 20, 22, 24, 25, 26, 27, 28, 29, 30, 31,
  39, 37, 35, 33, 32, 34, 36, 38, 40, 41, 42, 43, 44, 45, 46, 47
};

std::string XQFFile::genInitFen()
{
    int piecePos[32];

    if (_xqfHeader.Version < 12) {
        for (int i = 0; i < 32; i++) {
            piecePos[i] = _xqfHeader.QiziXY[i] - _pieceOffset;
        }
    } else {
        for (int i = 0; i < 32; i++) {
            piecePos[(_pieceOffset + 1 + i) % 32] = _xqfHeader.QiziXY[i] - _pieceOffset;
        }
    }

    SimplePosition::PositionStruct pos;
    pos.FromFen("");

    for (int i = 0; i < 32; i++) {
        log("pos[%d]: %02d", i, piecePos[i]);
        if (piecePos[i] < 90) {
            pos.AddPiece(cucsqXqf2Square[piecePos[i]], cpcXqf2Piece[i]);
        }
    }

    char buf[256];
    pos.ToFen(buf);

    return std::string(buf);
}
