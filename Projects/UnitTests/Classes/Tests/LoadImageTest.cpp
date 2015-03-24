#include "Tests/LoadImageTest.h"
#include "Infrastructure/GameCore.h"
#include "Render/Image/ImageSystem.h"

using namespace DAVA;

LoadImageTest::LoadImageTest()
: TestTemplate<LoadImageTest>("LoadImageTest")
{
    RegisterFunction(this, &LoadImageTest::TgaTest, "TgaTest", NULL);
}

void LoadImageTest::TgaTest(PerfFuncData * data)
{
    static const std::array<uint8, 4 * 10 * 10> tga10x10data = {
        0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x24, 0xff, 0x00, 0xff, 0x24, 0xff,
        0x00, 0xff, 0x24, 0xff, 0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0x24, 0xff, 0x00, 0xff, 0x24, 0xb9, 0x00, 0xff, 0x24, 0xb9, 0x12, 0x00, 0xff, 0xb9, 0x12, 0x00, 0xff, 0xb9, 0x12, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x24, 0xff, 0x00, 0xff, 0x24, 0xb9, 0x00, 0xff, 0x24, 0xb9, 0x12, 0x00, 0xff, 0xb9, 0x12, 0x00, 0xff, 0xb9, 0x12, 0x00, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xb9, 0x12, 0x00, 0xff, 0xb9, 0x00, 0xff, 0x24, 0xb9,
        0x00, 0xff, 0x24, 0xb9, 0x00, 0xff, 0x24, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xb9,
        0x12, 0x00, 0xff, 0xb9, 0x00, 0xff, 0x24, 0xb9, 0x00, 0xff, 0x24, 0xb9, 0x00, 0xff, 0x24, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0x12, 0x00, 0xff, 0xff, 0x00, 0xff, 0x24, 0xff, 0x00, 0xff, 0x24, 0xff, 0x00, 0xff, 0x24, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xf3, 0x00, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x00, 0x12, 0xff, 0xf3, 0x00, 0x12, 0xff };

    DAVA::Vector<DAVA::Image*> imgSet;

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load("~res:/TestData/LoadImageTest/rgb888_rle_topleft.tga", imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->GetPixelFormat() == PixelFormat::FORMAT_RGB888);
    imgSet.clear();

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load("~res:/TestData/LoadImageTest/rgba8888_rle_bottomleft.tga", imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->GetPixelFormat() == PixelFormat::FORMAT_RGBA8888);
    imgSet.clear();

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load("~res:/TestData/LoadImageTest/a8_norle_bottomleft.tga", imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->GetPixelFormat() == PixelFormat::FORMAT_A8);
    imgSet.clear();

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load("~res:/TestData/LoadImageTest/10x10_rgba8888.tga", imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->GetPixelFormat() == PixelFormat::FORMAT_RGBA8888);
    TEST_VERIFY(imgSet[0]->dataSize == 4 * 10 * 10);
    TEST_VERIFY(Memcmp(imgSet[0]->data, &tga10x10data, tga10x10data.size()) == 0);
    imgSet.clear();

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load("~res:/TestData/LoadImageTest/10x10_rgba8888_norle.tga", imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->GetPixelFormat() == PixelFormat::FORMAT_RGBA8888);
    TEST_VERIFY(imgSet[0]->dataSize == 4 * 10 * 10);
    TEST_VERIFY(Memcmp(imgSet[0]->data, &tga10x10data, tga10x10data.size()) == 0);
    imgSet.clear();
}
