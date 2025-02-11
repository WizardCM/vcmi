/*
 * mainwindow_moc.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "mainwindow_moc.h"
#include "ui_mainwindow_moc.h"

#include <QProcess>
#include <QDir>

#include "../lib/CConfigHandler.h"
#include "../lib/VCMIDirs.h"
#include "../lib/filesystem/Filesystem.h"
#include "../lib/logging/CBasicLogConfigurator.h"

#include "updatedialog_moc.h"

void MainWindow::load()
{
	// Set current working dir to executable folder.
	// This is important on Mac for relative paths to work inside DMG.
	QDir::setCurrent(QApplication::applicationDirPath());

	console = new CConsoleHandler();
	CBasicLogConfigurator logConfig(VCMIDirs::get().userCachePath() / "VCMI_Launcher_log.txt", console);
	logConfig.configureDefault();

	CResourceHandler::initialize();
	CResourceHandler::load("config/filesystem.json");

	for(auto & string : VCMIDirs::get().dataPaths())
		QDir::addSearchPath("icons", pathToQString(string / "launcher" / "icons"));
	QDir::addSearchPath("icons", pathToQString(VCMIDirs::get().userDataPath() / "launcher" / "icons"));

	settings.init();
}

MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	load(); // load FS before UI

	ui->setupUi(this);

	//load window settings
	QSettings s(Ui::teamName, Ui::appName);

	auto size = s.value("MainWindow/Size").toSize();
	if(size.isValid())
	{
		resize(size);
	}
	auto position = s.value("MainWindow/Position").toPoint();
	if(!position.isNull())
	{
		move(position);
	}

	//set default margins

	auto width = ui->startGameTitle->fontMetrics().boundingRect(ui->startGameTitle->text()).width();
	if(ui->startGameButton->iconSize().width() < width)
	{
		ui->startGameButton->setIconSize(QSize(width, width));
	}
	auto tab_icon_size = ui->tabSelectList->iconSize();
	if(tab_icon_size.width() < width)
	{
		ui->tabSelectList->setIconSize(QSize(width, width + tab_icon_size.height() - tab_icon_size.width()));
		ui->tabSelectList->setGridSize(QSize(width, width));
		// 4 is a dirty hack to make it look right
		ui->tabSelectList->setMaximumWidth(width + 4);
	}
	ui->tabListWidget->setCurrentIndex(0);
	ui->settingsView->setDisplayList();

	connect(ui->tabSelectList, SIGNAL(currentRowChanged(int)),
		ui->tabListWidget, SLOT(setCurrentIndex(int)));

	if(settings["launcher"]["updateOnStartup"].Bool())
		UpdateDialog::showUpdateDialog(false);
}

MainWindow::~MainWindow()
{
	//save window settings
	QSettings s(Ui::teamName, Ui::appName);
	s.setValue("MainWindow/Size", size());
	s.setValue("MainWindow/Position", pos());

	delete ui;
}

void MainWindow::on_startGameButton_clicked()
{
	startExecutable(pathToQString(VCMIDirs::get().clientPath()));
}

void MainWindow::startExecutable(QString name)
{
	QProcess process;

	// Start the executable
	if(process.startDetached(name, {}))
	{
		close(); // exit launcher
	}
	else
	{
		QMessageBox::critical(this,
		                      "Error starting executable",
		                      "Failed to start " + name + "\n"
		                      "Reason: " + process.errorString(),
		                      QMessageBox::Ok,
		                      QMessageBox::Ok);
		return;
	}
}
