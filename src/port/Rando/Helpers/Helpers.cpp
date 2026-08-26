#include "Helpers.h"
#include "port/UI/Notification.h"
#include "port/Rando/Logic/Logic.h"

namespace Rando {
namespace Helpers {

void SendNotification(RandoItemId randoItemId, const std::string& subject) {
    RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;
    actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    std::string prefix = "";
    std::string message = Rando::StaticData::Items[randoItemId].name;
    std::string suffix = "";
    ImVec4 itemColor = UIWidgets::ColorValues.at(Rando::StaticData::ItemColors[actorId]);

    if (itemType == RITYPE_MOLEHILL) {
        prefix = subject + " learned";
    } else if (itemType == RITYPE_SNS_EGG || itemType == RITYPE_SNS_KEY) {
        int32_t totalsnsItems = Rando::Logic::GetTotalSnsItemsCollected();
        prefix = subject + " collected ";
        prefix += Rando::StaticData::Items[randoItemId].article;
        suffix = "(";
        suffix += std::to_string(totalsnsItems);
        suffix += " / 7)";

        itemColor = UIWidgets::ColorValues.at(Rando::StaticData::SnsColors[randoItemId]);
    } else {
        prefix = subject + " collected ";
        prefix += Rando::StaticData::Items[randoItemId].article;
    }

    Notification::Emit({
        .prefix = prefix,
        .prefixColor = UIWidgets::ColorValues.at(UIWidgets::Colors::White),
        .message = message,
        .messageColor = itemColor,
        .suffix = suffix,
        .suffixColor = UIWidgets::ColorValues.at(UIWidgets::Colors::White),
    });
}

} // namespace Helpers
} // namespace Rando
