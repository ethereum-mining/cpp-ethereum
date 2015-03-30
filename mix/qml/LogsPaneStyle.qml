pragma Singleton
import QtQuick 2.0

QtObject {

	function absoluteSize(rel)
	{
		return systemPointSize + rel;
	}

	property QtObject generic: QtObject {
		property QtObject layout: QtObject {
			property string backgroundColor: "#f7f7f7"
			property int headerHeight: 35
			property int headerButtonSpacing: 5
			property int leftMargin: 10
			property int headerButtonHeight: 30
			property string logLabelColor: "#5391d8"
			property string logLabelFont: "sans serif"
			property int headerInputWidth: 200
			property int dateWidth: 70
			property int typeWidth: 70
			property int contentWidth: 250
			property string logAlternateColor: "#f0f0f0"
		}
	}
}
