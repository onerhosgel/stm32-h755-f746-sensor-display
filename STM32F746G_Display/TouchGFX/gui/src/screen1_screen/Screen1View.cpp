#include <gui/screen1_screen/Screen1View.hpp>
#include <texts/TextKeysAndLanguages.hpp>

extern "C"
{
    extern volatile uint32_t g_sensorPacketCount;
    extern volatile int32_t g_temperature10;
    extern volatile int32_t g_humidity10;
    extern volatile int32_t g_pressure10;
    extern volatile uint16_t g_red;
    extern volatile uint16_t g_green;
    extern volatile uint16_t g_blue;
    extern volatile uint16_t g_clear;
}

Screen1View::Screen1View()
    : x_degeri(2),
      saniye_sayaci(0),
      calisiyor_mu(true)
{
}

Screen1View::~Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    ekraniGuncelle(x_degeri);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::butonBasildi()
{
    calisiyor_mu = !calisiyor_mu;
}

void Screen1View::handleTickEvent()
{
    static uint32_t lastPacketCount = 0U;
    static uint8_t sensorPage = 0U;
    bool refreshNow = false;

    /* Sensör paketi henüz gelmediyse eski sayaç devam etsin. */
    if (g_sensorPacketCount == 0U)
    {
        if (!calisiyor_mu)
        {
            return;
        }

        saniye_sayaci++;

        if (saniye_sayaci >= 120)
        {
            saniye_sayaci = 0;

            if (x_degeri >= 20)
            {
                x_degeri = 0;
            }
            else
            {
                x_degeri += 2;
            }

            ekraniGuncelle(x_degeri);
        }

        return;
    }

    /* Yeni paket gelince mevcut sayfayı hemen güncelle. */
    if (g_sensorPacketCount != lastPacketCount)
    {
        lastPacketCount = g_sensorPacketCount;
        refreshNow = true;
    }

    /* Her yaklaşık 2 saniyede bir sonraki sensör değerine geç. */
    if (calisiyor_mu)
    {
        saniye_sayaci++;

        if (saniye_sayaci >= 120)
        {
            saniye_sayaci = 0;
            sensorPage++;

            if (sensorPage >= 7U)
            {
                sensorPage = 0U;
            }

            refreshNow = true;
        }
    }

    if (!refreshNow)
    {
        return;
    }

    switch (sensorPage)
    {
    case 0:
    {
        int value = (int)g_temperature10;
        int absValue = (value < 0) ? -value : value;

        if (value < 0)
        {
            Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                              "T-%d.%d", absValue / 10, absValue % 10);
        }
        else
        {
            Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                              "T%d.%d", absValue / 10, absValue % 10);
        }
        break;
    }

    case 1:
    {
        int value = (int)g_humidity10;
        int absValue = (value < 0) ? -value : value;

        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "H%d.%d", absValue / 10, absValue % 10);
        break;
    }

    case 2:
    {
        int value = (int)g_pressure10;
        int absValue = (value < 0) ? -value : value;

        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "P%d.%d", absValue / 10, absValue % 10);
        break;
    }

    case 3:
        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "R%u", (unsigned int)g_red);
        break;

    case 4:
        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "G%u", (unsigned int)g_green);
        break;

    case 5:
        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "B%u", (unsigned int)g_blue);
        break;

    default:
        Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE,
                          "C%u", (unsigned int)g_clear);
        break;
    }

    textArea1.invalidate();
}

void Screen1View::ekraniGuncelle(int deger)
{
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%d", deger);
    textArea1.invalidate();
}
