
bool MessageBox::init()
{
	if (!Layout::init())
		return false;

	setLayoutType(Layout::Type::VERTICAL);

	_titleText = Text::create();
	_contentText = Text::create();

	return true;
}

void MessageBox::setTitle(std::string title)
{
	_titleText->setString(title)
}

void MessageBox::setContent(std::string content)
{
	_contentText->setString(title)
}

void MessageBox::addButton(const std::string& normalImage,
		const std::string& selectedImage,
		const std::string& disableImage)
{
	auto button = Button::create(normalImage, selectedImage, disableImage);
}

