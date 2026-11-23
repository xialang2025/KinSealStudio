#ifndef KEYBOARD_PREVIEW_H
#define KEYBOARD_PREVIEW_H

#include <QString>
#include <optional>

class QWidget;
struct HmiProject;

//20261123-26 add: Return the accepted preview text without writing it to the editor or an address.
std::optional<QString> showStage2KeyboardPreview(QWidget *parent, const HmiProject &project);

#endif
