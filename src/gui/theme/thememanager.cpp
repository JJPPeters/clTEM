//
// Created by Jon on 12/06/2018.
//

#include "thememanager.h"

ThemeManager::Theme ThemeManager::CurrentTheme = ThemeManager::Theme::Native;

//
// Dark theme
//
ThemeColours ThemeManager::DarkTheme = ThemeColours("dark",
                                                    "#30343F",
                                                    "#3F4551",
                                                    "#4E5665",
                                                    "#D8D8D8",
                                                    "#30343F",
                                                    "#353945",
                                                    "#3A3F4B",
                                                    "#6D7483");

//
// Mid Dark theme
//
ThemeColours ThemeManager::MidDarkTheme = ThemeColours("dark",
                                                       "#455564",
                                                       "#38444F",
                                                       "#3F5060",
                                                       "#D8D8D8",
                                                       "#D8D8D8",
                                                       "#4E6071",
                                                       "#3F4E5C",
                                                       "#8094A6");

//
// Mid Light theme
//
ThemeColours ThemeManager::MidLightTheme = ThemeColours("light",
                                                        "#647C84",
                                                        "#879497",
                                                        "#6C868F",
                                                        "#273645",
                                                        "#2A333C",
                                                        "#A7AEB2",
                                                        "#9EA6AA",
                                                        "#B2BDC4");

//
// Light theme
//
ThemeColours ThemeManager::LightTheme = ThemeColours("light",
                                                       "#E6EBEE",
                                                       "#A6B1B7",
                                                       "#A1ADB3",
                                                       "#3B4854",
                                                       "#E6EBEE",
                                                       "#FFFFFF",
                                                       "#B3BCC1",
                                                       "#8C9CA6");