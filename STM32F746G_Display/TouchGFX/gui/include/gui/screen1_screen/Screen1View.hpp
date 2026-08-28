#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View();
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void handleTickEvent();

    // Designer'da yazdığımız fonksiyon ismini buraya da yazıyoruz:
    virtual void butonBasildi();

protected:
    int x_degeri;
    int saniye_sayaci;
    
    // --- START / STOP ANAHTARIMIZ ---
    bool calisiyor_mu; 

    void ekraniGuncelle(int deger);
};

#endif // SCREEN1VIEW_HPP