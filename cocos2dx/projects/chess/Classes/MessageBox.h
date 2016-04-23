#ifndef __MESSAGEBOX_H__
#define __MESSAGEBOX_H__

class MessageBox : public Layout
{
public:
	virtual bool init();
	CREATE_FUNC(MessageBox);

	void setTitle(std::string title);
	void setContent(std::string content);
	void addButton(const std::string& normalImage,
			const std::string& selectedImage = "",
			const std::string& disableImage = "");
	int getCurSelectedIndex();
	Widget *getButton(int index);
	Widget *getCurSelectedButton();

private:
	int _curSelectedIndex;
	std::vector<Button *> _buttons;
	Text *_titleText;
	Text *_contentText;
};

#endif
