#pragma once

#include <QtGui/QColor>

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QUuid>
#include <QtCore/QVersionNumber>

struct TaskHeader
{
    QUuid id;
    QString name;
    QColor color;
    QDateTime created;

    // bool operator==(const TaskHeader &other) const = default;
    bool operator==(const TaskHeader &other) const noexcept
    {
        return id == other.id
               && name == other.name
               && color == other.color
               && created == other.created;
    }
};

struct Task
{
    enum class Priority {
        Low = 0,
        Medium = 1,
        High = 2,
        Critical = 3,
    };

    TaskHeader header;
    Priority priority;
    bool completed;
    QString description;

    // bool operator==(const Task &other) const = default;
    bool operator==(const Task &other) const noexcept
    {
        return header == other.header
               && priority == other.priority
               && completed == other.completed
               && description == other.description;
    }
};

struct TaskList
{
    TaskHeader header;
    QList<Task> tasks;

    // bool operator==(const TaskList &other) const = default;
    bool operator==(const TaskList &other) const noexcept
    {
        return header == other.header
               && tasks == other.tasks;
    }
};

struct TaskManager
{
    QString user;
    QVersionNumber version;
    QList<TaskList> lists;

    // bool operator==(const TaskManager &other) const = default;
    bool operator==(const TaskManager &other) const noexcept
    {
        return user == other.user
               && version == other.version
               && lists == other.lists;
    }
};

