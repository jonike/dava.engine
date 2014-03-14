#include "StringFormatTest.h"

using namespace DAVA;

StringFormatTest::~StringFormatTest()
{

}

StringFormatTest::StringFormatTest()
    : TestTemplate<StringFormatTest>("StringFormatTest")
{
    RegisterFunction(this, &StringFormatTest::StringTestFunction, "StringTestFunction", NULL);
    RegisterFunction(this, &StringFormatTest::IntegerTestFunction, "IntegerTestFunction", NULL);
    RegisterFunction(this, &StringFormatTest::FloatTestFunction, "FloatTestFunction", NULL);
}

void StringFormatTest::LoadResources()
{

}

void StringFormatTest::UnloadResources()
{

}

void StringFormatTest::StringTestFunction( PerfFuncData * data )
{
//     WideString formatStr1 = L"%ls %ls";
//     WideString value1 = L"test string";
//     WideString value2 = L"second";
//     TEST_VERIFY( Format( formatStr1.c_str(), value1.c_str(), value2.c_str()) == StringToWString( Format( WStringToString(formatStr1).c_str(), WStringToString(value1).c_str(), WStringToString(value2).c_str() ) ) );
}

void StringFormatTest::IntegerTestFunction( PerfFuncData * data )
{
    WideString formatStr1 = L"%i%%"  ;
    WideString formatStr2 = L"%d%%"  ;
    WideString formatStr3 = L"%lld%%";
    int32 value = 1234567890;
    int64 value64 = 1234567890123456789;
    TEST_VERIFY( Format(formatStr1.c_str(), value) == StringToWString( Format( WStringToString(formatStr1).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr2.c_str(), value) == StringToWString( Format( WStringToString(formatStr2).c_str(), value ) ) );
    TEST_VERIFY( Format(formatStr3.c_str(), value64) == StringToWString( Format( WStringToString(formatStr3).c_str(), value64 ) ) );
}

void StringFormatTest::FloatTestFunction( PerfFuncData * data )
{
    WideString formatStr1 = L"%f"   ;
    WideString formatStr2 = L"%6.f";
    WideString formatStr3 = L"%5.f";
    WideString formatStr4 = L"%4.f";
    WideString formatStr5 = L"%3.f";
    WideString formatStr6 = L"%2.f";
    WideString formatStr7 = L"%1.f";
    WideString formatStr8 = L"%0.f";
    WideString formatStr9 = L"%.6f";
    WideString formatStr10= L"%.5f";
    WideString formatStr11= L"%.4f";
    WideString formatStr12= L"%.3f";
    WideString formatStr13= L"%.2f";
    WideString formatStr14= L"%.1f";
    WideString formatStr15= L"%.0f";
    WideString formatStr16= L"%6.6f";
    WideString formatStr17= L"%5.5f";
    WideString formatStr18= L"%4.4f";
    WideString formatStr19= L"%3.3f";
    WideString formatStr20= L"%2.2f";
    WideString formatStr21= L"%1.1f";
    WideString formatStr22= L"%0.0f";
    float32 value1 = 1234.1234f;
    float32 value2 = 876.876f;

    TEST_VERIFY( Format(formatStr1.c_str() , value1) == StringToWString( Format( WStringToString(formatStr1 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr2.c_str() , value1) == StringToWString( Format( WStringToString(formatStr2 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr3.c_str() , value1) == StringToWString( Format( WStringToString(formatStr3 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr4.c_str() , value1) == StringToWString( Format( WStringToString(formatStr4 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr5.c_str() , value1) == StringToWString( Format( WStringToString(formatStr5 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr6.c_str() , value1) == StringToWString( Format( WStringToString(formatStr6 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr7.c_str() , value1) == StringToWString( Format( WStringToString(formatStr7 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr8.c_str() , value1) == StringToWString( Format( WStringToString(formatStr8 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr9.c_str() , value1) == StringToWString( Format( WStringToString(formatStr9 ).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr10.c_str(), value1) == StringToWString( Format( WStringToString(formatStr10).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr11.c_str(), value1) == StringToWString( Format( WStringToString(formatStr11).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr12.c_str(), value1) == StringToWString( Format( WStringToString(formatStr12).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr13.c_str(), value1) == StringToWString( Format( WStringToString(formatStr13).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr14.c_str(), value1) == StringToWString( Format( WStringToString(formatStr14).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr15.c_str(), value1) == StringToWString( Format( WStringToString(formatStr15).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr16.c_str(), value1) == StringToWString( Format( WStringToString(formatStr16).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr17.c_str(), value1) == StringToWString( Format( WStringToString(formatStr17).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr18.c_str(), value1) == StringToWString( Format( WStringToString(formatStr18).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr19.c_str(), value1) == StringToWString( Format( WStringToString(formatStr19).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr20.c_str(), value1) == StringToWString( Format( WStringToString(formatStr20).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr21.c_str(), value1) == StringToWString( Format( WStringToString(formatStr21).c_str(), value1) ) );
    TEST_VERIFY( Format(formatStr22.c_str(), value1) == StringToWString( Format( WStringToString(formatStr22).c_str(), value1) ) );
    
    TEST_VERIFY( Format(formatStr1.c_str() , value2) == StringToWString( Format( WStringToString(formatStr1 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr2.c_str() , value2) == StringToWString( Format( WStringToString(formatStr2 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr3.c_str() , value2) == StringToWString( Format( WStringToString(formatStr3 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr4.c_str() , value2) == StringToWString( Format( WStringToString(formatStr4 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr5.c_str() , value2) == StringToWString( Format( WStringToString(formatStr5 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr6.c_str() , value2) == StringToWString( Format( WStringToString(formatStr6 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr7.c_str() , value2) == StringToWString( Format( WStringToString(formatStr7 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr8.c_str() , value2) == StringToWString( Format( WStringToString(formatStr8 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr9.c_str() , value2) == StringToWString( Format( WStringToString(formatStr9 ).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr10.c_str(), value2) == StringToWString( Format( WStringToString(formatStr10).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr11.c_str(), value2) == StringToWString( Format( WStringToString(formatStr11).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr12.c_str(), value2) == StringToWString( Format( WStringToString(formatStr12).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr13.c_str(), value2) == StringToWString( Format( WStringToString(formatStr13).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr14.c_str(), value2) == StringToWString( Format( WStringToString(formatStr14).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr15.c_str(), value2) == StringToWString( Format( WStringToString(formatStr15).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr16.c_str(), value2) == StringToWString( Format( WStringToString(formatStr16).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr17.c_str(), value2) == StringToWString( Format( WStringToString(formatStr17).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr18.c_str(), value2) == StringToWString( Format( WStringToString(formatStr18).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr19.c_str(), value2) == StringToWString( Format( WStringToString(formatStr19).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr20.c_str(), value2) == StringToWString( Format( WStringToString(formatStr20).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr21.c_str(), value2) == StringToWString( Format( WStringToString(formatStr21).c_str(), value2) ) );
    TEST_VERIFY( Format(formatStr22.c_str(), value2) == StringToWString( Format( WStringToString(formatStr22).c_str(), value2) ) );
 
    float32 value3 = 0.1234f;
    float32 value4 = 0.2567f;
    float32 value5 = 0.5f;
    float32 value6 = 0.7543f;
    TEST_VERIFY( Format(formatStr15.c_str(), value3) == StringToWString( Format( WStringToString(formatStr15).c_str(), value3) ) );
    TEST_VERIFY( Format(formatStr15.c_str(), value4) == StringToWString( Format( WStringToString(formatStr15).c_str(), value4) ) );
    TEST_VERIFY( Format(formatStr15.c_str(), value5) == StringToWString( Format( WStringToString(formatStr15).c_str(), value5) ) );
    TEST_VERIFY( Format(formatStr15.c_str(), value6) == StringToWString( Format( WStringToString(formatStr15).c_str(), value6) ) );

}
