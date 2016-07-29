#ifndef __XQFFILE_H__
#define __XQFFILE_H__

#include "cocos2d.h"
#include "XQFile.h"

USING_NS_CC;

struct XQFFileHeader
{
    uint16_t Signature;
    uint8_t Version;
    uint8_t KeyMask;
    uint32_t ProductId;
    uint8_t KeyOrA;
    uint8_t KeyOrB;
    uint8_t KeyOrC;
    uint8_t KeyOrD;
    uint8_t KeysSum;
    uint8_t KeyXY;
    uint8_t KeyXYf;
    uint8_t KeyXYt;
    uint8_t QiziXY[32];
    uint16_t PlayStepNo;
    uint8_t WhoPlay;
    uint8_t PlayResult; // offset 0x33
    uint32_t PlayNodes;
    uint32_t PTreePos;
    uint8_t Reserved1[4];
    uint16_t CodeA; // 棋局类型(开中残全), offset: 0x40
    uint16_t CodeB;
    uint16_t CodeC;
    uint16_t CodeD;
    uint16_t CodeE;
    uint16_t CodeF;
    uint16_t CodeH;
    uint16_t CodeG;
    uint8_t TitleLen; // 标题名称 offset 0x50
    int8_t Title[63];
    uint8_t TitleBLen;
    int8_t TitleB[63];
    uint8_t MatchNameLen; // 比赛名称 offset 0xD0
    int8_t MatchName[63]; // 比赛名称
    uint8_t DateLen; // 比赛日期, offset 0x110
    int8_t Date[15];
    uint8_t AddrLen; // 比赛地点，offset 0x120
    int8_t Addr[15];
    uint8_t RedPlayerLen; // 红方棋手姓名, offset 0x130
    int8_t RedPlayer[15];
    uint8_t BlackPlayerLen; // 黑方棋手姓名， offset 0x140
    int8_t BlackPlayer[15];
    uint8_t TimeRuleLen; //　用时规则, offset 0x151
    int8_t TimeRule[63];
    uint8_t RedTimeLen; // 红方用时长度, offset 0x190
    int8_t RedTime[15];
    uint8_t BlackTimeLen; // 黑方用时长度, offset 0x1a0
    int8_t BlackTime[15];
    uint8_t Reserved2[32];
    uint8_t AnnotatorLen; // 讲评人姓名长度, offset 0x1D0
    int8_t Annotator[15];
    uint8_t AuthorLen; // 文件作者姓名长度, offset 0x1E0
    int8_t Author[15];
    uint8_t Reserved3[16];
    uint8_t Reserved4[512];
} __attribute__((packed));

class XQFFile : public XQFile
{
public:
    void load(std::string name) override;

private:
    XQNode *readStep(XQNode *parent);
    void readAndDecrypt(void *buf, int len);
    void initXQHeader();
    bool checkFormat();
    void initDecryptKey();
    std::string genInitFen();

    Data _xqfData;

    XQFFileHeader _xqfHeader;

    int _pieceOffset;
    int _srcOffset;
    int _dstOffset;
    int _commentOffset;
    int _encKey[32];
    int _fileOffset;
};

#endif
