/**
* The MIT License
 *
 * Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Last updated: July 29, 2024
 */

#include "../../sunnypilot/selfdrive/ui/qt/offroad/settings/onboarding.h"

#include <string>

#include <QScrollBar>
#include <common/swaglog.h>

#include "common/util.h"
#include "common/params.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

void TermsPageSP::showEvent(QShowEvent *event) {
  // late init, building QML widget takes 200ms
  if (layout()) {
    return;
  }

  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(45, 35, 45, 45);
  main_layout->setSpacing(0);

  QLabel *title = new QLabel(tr("Terms & Conditions"));
  title->setStyleSheet("font-size: 90px; font-weight: 600;");
  main_layout->addWidget(title);

  QLabel *text = new QLabel(this);
  text->setTextFormat(Qt::RichText);
  text->setWordWrap(true);
  std::string tc_text = sunnypilot_tc ? "../assets/offroad/sp_tc.html" : "../assets/offroad/tc.html";
  text->setText(QString::fromStdString(util::read_file(tc_text)));
  text->setStyleSheet("font-size:50px; font-weight: 200; color: #C9C9C9; background-color:#1B1B1B; padding:50px 50px;");
  ScrollView *scroll = new ScrollView(text, this);

  main_layout->addSpacing(30);
  main_layout->addWidget(scroll);
  main_layout->addSpacing(50);

  QHBoxLayout *buttons = new QHBoxLayout;
  buttons->setMargin(0);
  buttons->setSpacing(45);
  main_layout->addLayout(buttons);

  QPushButton *decline_btn = new QPushButton(tr("Decline"));
  buttons->addWidget(decline_btn);
  QObject::connect(decline_btn, &QPushButton::clicked, this, &TermsPage::declinedTerms);

  accept_btn = new QPushButton(tr("Scroll to accept"));
  accept_btn->setEnabled(false);
  accept_btn->setStyleSheet(R"(
    QPushButton {
      background-color: #465BEA;
    }
    QPushButton:pressed {
      background-color: #3049F4;
    }
    QPushButton:disabled {
      background-color: #4F4F4F;
    }
  )");
  buttons->addWidget(accept_btn);
  QObject::connect(accept_btn, &QPushButton::clicked, this, &TermsPage::acceptedTerms);
  QScrollBar *scroll_bar = scroll->verticalScrollBar();
  connect(scroll_bar, &QScrollBar::valueChanged, this, [this, scroll_bar](int value) {
    if (value == scroll_bar->maximum()) {
      enableAccept();
    }
  });
}

void OnboardingWindowSP::updateActiveScreen() {
  if (accepted_terms && training_done && !accepted_terms_sp) {
    setCurrentIndex(3);
  } else {
    OnboardingWindow::updateActiveScreen();
  }
}

OnboardingWindowSP::OnboardingWindowSP(QWidget *parent) : OnboardingWindow(parent) {
  std::string current_terms_version_sp = params.get("TermsVersionSunnypilot");
  accepted_terms_sp = params.get("HasAcceptedTermsSP") == current_terms_version_sp;
  LOGD("accepted_terms_sp: %s", params.get("HasAcceptedTermsSP").c_str());

  auto *terms_sp = new TermsPageSP(true, parent);
  addWidget(terms_sp); // index = 3
  connect(terms_sp, &TermsPageSP::acceptedTerms, [=]() {
    params.put("HasAcceptedTermsSP", current_terms_version_sp);
    accepted_terms_sp = true;
    updateActiveScreen();
  });
  connect(terms_sp, &TermsPageSP::declinedTerms, [=]() { setCurrentIndex(2); });

  OnboardingWindowSP::updateActiveScreen();
}
