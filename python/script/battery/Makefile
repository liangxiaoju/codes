rmfile = build dist *.pyc *.spec images_rc.py *.log

all:
	pyrcc4 -o images_rc.py images.qrc
	pyinstaller -w -F ui.py
	mv dist/ui pybattery
	rm -rf $(rmfile)

clean:
	rm -rf $(rmfile)
